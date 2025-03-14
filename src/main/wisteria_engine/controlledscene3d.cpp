#include <iostream>
#include <filesystem>
#include "gameobject3d.h"
#include "controlledscene3d.h"
#include "transform3d.h"
#include "camera.h"
#include "material.h"
#include "assets.h"

#define CAMERA_INIT_FOVY 60
#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720
#define CAMERA_INIT_ZNEAR 0.01f
#define CAMERA_INIT_ZFAR 300.0f
#define DEFAULT_LAYER 0

namespace fs = std::filesystem;

using namespace engine;

void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
    std::abort();
}

ControlledScene3D::ControlledScene3D()
{
    gameObjects.reserve(70);
    toDestroy.reserve(10);
    // usually, there aren't more than 2 cameras in a scene
    cameras.reserve(2);
    layers.assign(32, std::unordered_set<GameObject *>());
    lights.reserve(10);
    collisionMasks.assign(32, 0);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
}

ControlledScene3D::~ControlledScene3D()
{
    for (auto &gameObject : gameObjects)
        delete gameObject;

    gameObjects.clear();
    toDestroy.clear();
    cameras.clear();
}

void engine::checkFBStatus()
{
    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE)
        return;

    std::cerr << "Framebuffer is not complete: " << status << std::endl;
    switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            std::cerr << "GL_FRAMEBUFFER_UNDEFINED" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
            break;
        default:
            std::cerr << "Unknown error" << std::endl;
            break;
    }
}

void ControlledScene3D::Init()
{
    // ignore the fact that SimpleScene already provides a camera. In the future,
    // I plan on removing the entire framework and replacing it with my own.
    GetCameraInput()->SetActive(false);
    mainCamera = new Camera(glm::vec3(0, 1.4, 0), glm::vec3_forward, glm::vec3_up);
    mainCamera->SetPerspective(CAMERA_INIT_FOVY,
                               DEFAULT_WINDOW_WIDTH / (float)DEFAULT_WINDOW_HEIGHT,
                               CAMERA_INIT_ZNEAR, CAMERA_INIT_ZFAR);
    cameras.push_back(mainCamera);

    windowResolution = window->GetResolution();

    InitMeshes();
    InitShaders();
    Assets::lookupDirectory = window->props.selfDir;

    this->Initialize();
    ResizeDrawArea();
}

void ControlledScene3D::InitMeshes()
{
    Assets::lookupDirectory = window->props.selfDir;
    std::string meshes = PATH_JOIN(SOURCE_PATH::MAIN, "wisteria_engine", "meshes");
    Assets::LoadMesh("Default/Cube", meshes, "cube.obj");
    Assets::LoadMesh("Default/Sphere", meshes, "sphere.obj");
    Assets::LoadMesh("Default/Quad", meshes, "quad.obj");
}

void ControlledScene3D::InitShaders()
{
    Assets::lookupDirectory = PATH_JOIN(window->props.selfDir, SOURCE_PATH::MAIN, "wisteria_engine", "shaders");
    try {
        for (const auto &entry : fs::directory_iterator(Assets::lookupDirectory)) {
            if (entry.is_regular_file()) {
                Assets::AddPath(entry.path().stem().string(), entry.path().filename().string());
            }
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        exit(1);
    }

    Assets::LoadShader("VertexColor", "Default.VS", "Default.VertexColor.FS");
    Assets::LoadShader("NormalColor", "Default.VS", "Default.NormalColor.FS");
    Assets::LoadShader("PlainColor", "Default.VS", "PlainColor.FS");
    Assets::LoadShader("Texture", "Default.VS", "Default.Texture.FS");
    Assets::LoadShader("TransformTexture", "Transform.Texture.VS", "Default.Texture.FS");
    Assets::LoadShader("Skybox", "Skybox.VS", "Skybox.FS");
    Assets::LoadShader("AllData", "Default.VS", "Default.All.FS");
    Assets::LoadShader("Deffered/LightAccumulate", "Default.VS", "Deffered.Light.FS");
    Assets::LoadShader("Deffered/Composite", "ScreenSpace.VS", "Deffered.Composite.FS");
    Assets::LoadShader("Deffered/LightAccumulate/Cube", "Default.VS", "Deffered.Light.Cube.FS");
    Assets::LoadShader("Deffered/Composite/Cube", "ScreenSpace.VS", "Deffered.Composite.Cube.FS");
    Assets::LoadShader("Particles", "Particles.VS", "Particles.GS", "Default.Texture.FS");
}

void ControlledScene3D::AddToScene(GameObject *gameObject)
{
    // I don't think the perfomance cost of dynamic_cast here is significant
    if (dynamic_cast<Light *>(gameObject) != nullptr) {
        lights.push_back(static_cast<Light *>(gameObject));
    } else {
        gameObjects.insert(gameObject);
    }
    gameObject->scene = this;
    gameObject->Initialize();
    layers[DEFAULT_LAYER].insert(gameObject);
    for (auto &child : gameObject->GetChildren()) {
        AddToScene(child);
    }
}

void ControlledScene3D::Destroy(GameObject *gameObject)
{
    toDestroy.insert(gameObject);
    for (auto &child : gameObject->GetChildren()) {
        Destroy(child);
    }
}

void ControlledScene3D::AddToLayer(GameObject *gameObject, int layer)
{
    if (layer > 32) {
        std::cerr << "Wisteria Engine only supports 32 layers.\n";
        exit(1);
    }
    layers[layer].insert(gameObject);
    gameObject->layerMask |= (1 << layer);
}

void ControlledScene3D::RemoveFromLayer(GameObject *gameObject, int layer)
{
    if (layer > 32) {
        std::cerr << "Wisteria Engine only supports 32 layers.\n";
        exit(1);
    }
    layers[layer].erase(gameObject);
    gameObject->layerMask &= ~(1 << layer);
}

struct pair_hash {
    inline std::size_t operator()(const std::pair<GameObject *, GameObject *> &v) const {
        return reinterpret_cast<uintptr_t>(v.first) ^ reinterpret_cast<uintptr_t>(v.second);
    }
};

void ControlledScene3D::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ControlledScene3D::Update(float deltaTimeSeconds)
{
    deltaTime = deltaTimeSeconds * timeScale;
    unscaledDeltaTime = deltaTimeSeconds;

    Camera *savedMainCamera = mainCamera;
    std::set<Camera *> screenCameras;
    bool update = true;
    
    // cameras rendering into a framebuffer first
    for (auto &camera : cameras) {
        if (!camera->renderTarget) {
            screenCameras.insert(camera);
            continue;
        }
        mainCamera = camera;
        if (defferedRendering) {
            DefferedRenderScene(update);
            // renderTarget = camera->renderTarget;
            // ForwardRenderScene();
        } else {
            renderTarget = camera->renderTarget;
            ForwardRenderScene(update);
        }
        update = false;
    }
    // then screen cameras
    for (auto &camera : screenCameras) {
        mainCamera = camera;
        if (defferedRendering) {
            DefferedRenderScene(update);
        } else {
            renderTarget = camera->renderTarget;
            ForwardRenderScene(update);
        }
        update = false;
    }
    mainCamera = savedMainCamera;
    screenCameras.clear();

    Tick();

    CheckCollisions();
    
    for (auto gameObject : toDestroy) {
        if (gameObjects.find(gameObject) == gameObjects.end())
            continue;

        gameObjects.erase(gameObject);
        for (int layer = 0; layer < 32; ++layer)
            RemoveFromLayer(gameObject, layer);

        delete gameObject;
    }
    toDestroy.clear();
}

void ControlledScene3D::ForwardRenderScene(bool update)
{
    // std::cout << "Forward rendering from " << mainCamera->name << std::endl;
    if (!mainCamera->active)
        return;
        
    GLint vx = drawAreaX + (int)(mainCamera->viewportX * drawAreaWidth);
    GLint vy = drawAreaY + (int)(mainCamera->viewportY * drawAreaHeight);
    GLint vw = (int)(mainCamera->viewportWidth * drawAreaWidth);
    GLint vh = (int)(mainCamera->viewportHeight * drawAreaHeight);

    if (renderTarget) {
        renderTarget->Clear();
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->fbo);
    } else {
        glViewport(vx, vy, vw, vh);
    }

    for (auto gameObject : gameObjects) {
        if (!gameObject->active)
            continue;

        if (gameObject->GetLayerMask() & mainCamera->cullingMask)
            DrawGameObject(gameObject, update);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ControlledScene3D::DefferedRenderScene(bool update)
{
    gBuffer = mainCamera->gBuffer;

    // G-Buffer pass
    renderTarget = gBuffer;
    ForwardRenderScene(update);

    // Light accumulation pass
    gBuffer->Clear(false, true, { 3 });
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer->fbo);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        for (auto &light : lights) {
            renderTarget = gBuffer;
            AccumulateLight(light, update);
        }

        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    renderTarget = mainCamera->renderTarget;
    if (renderTarget == nullptr) {
        GLint vx = drawAreaX + (int)(mainCamera->viewportX * drawAreaWidth);
        GLint vy = drawAreaY + (int)(mainCamera->viewportY * drawAreaHeight);
        GLint vw = (int)(mainCamera->viewportWidth * drawAreaWidth);
        GLint vh = (int)(mainCamera->viewportHeight * drawAreaHeight);
        glViewport(vx, vy, vw, vh);
    } else {
        renderTarget->Clear();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget ? renderTarget->fbo : 0);

    // Composite pass
    Mesh *quad = Assets::meshes["Default/Quad"];
    Shader *shader = HelperDefferedShader("Deffered/Composite");

    Material material(shader);
    material.SetVec3("WIST_AMBIENT_LIGHT", Light::ambientLight);
    material.SetTexture("TEXTURE_COLOR", gBuffer->GetColorTexture(0));
    material.SetTexture("TEXTURE_LIGHT", gBuffer->GetColorTexture(3));
    material.Use();
    RenderMesh(quad, shader, 1, glm::mat4(1));
}

void ControlledScene3D::DrawGameObject(GameObject *gameObject, bool update)
{
    float objectDeltaTime = gameObject->useUnscaledTime ? unscaledDeltaTime : deltaTime;
    // if (update) gameObject->Tick(objectDeltaTime);
    gameObject->Tick(objectDeltaTime);

    if (gameObject->mesh) {
        glm::mat4 modelMatrix = gameObject->ObjectToWorldMatrix();
        if (gameObject->material.shader) {
            RenderMeshCustomMaterial(gameObject->mesh, gameObject->material, modelMatrix);
        } else {
            Shader *shader = defferedRendering ? Assets::shaders["AllData"] : Assets::shaders["VertexColor"];
            RenderMesh(gameObject->mesh, shader, 1, modelMatrix);
        }
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);  // in case Tick() bound an SSBO

    for (auto &child : gameObject->GetChildren()) {
        if (!child->active)
            continue;

        if (child->GetLayerMask() & mainCamera->cullingMask)
            DrawGameObject(child);
    }
}

void ControlledScene3D::RenderMesh(Mesh *mesh, Shader *shader, int instances, const glm::mat4 &modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    GLuint loc_view_matrix = glGetUniformLocation(shader->program, "WIST_VIEW_MATRIX");
    GLuint loc_projection_matrix = glGetUniformLocation(shader->program, "WIST_PROJECTION_MATRIX");
    GLuint loc_model_matrix = glGetUniformLocation(shader->program, "WIST_MODEL_MATRIX");
    GLuint loc_mvp_matrix = glGetUniformLocation(shader->program, "WIST_MVP");
    GLuint loc_eye_pos = glGetUniformLocation(shader->program, "WIST_EYE_POSITION");
    GLuint loc_resolution = glGetUniformLocation(shader->program, "WIST_RESOLUTION");
    GLuint loc_delta_time = glGetUniformLocation(shader->program, "WIST_DELTA_TIME");

    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(mainCamera->GetProjectionMatrix()));
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    if (loc_mvp_matrix != -1) {
        // only compute the MVP matrix if the shader uses it
        glm::mat4 mvp = mainCamera->GetProjectionMatrix() * mainCamera->GetViewMatrix() * modelMatrix;
        glUniformMatrix4fv(loc_mvp_matrix, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    if (loc_eye_pos != -1) {
        glUniform4fv(loc_eye_pos, 1, glm::value_ptr(mainCamera->GetPositionGeneralized()));
    }
    if (loc_resolution != -1) {
        glm::ivec2 resolution = renderTarget ? 
            glm::ivec2(renderTarget->GetWidth(), renderTarget->GetHeight()) :
            glm::ivec2(drawAreaWidth, drawAreaHeight);
        glUniform2iv(loc_resolution, 1, glm::value_ptr(resolution));
    }
    glUniform1f(loc_delta_time, deltaTime);

    mesh->UseMaterials(false);  // To whoever wrote gfxc: I hate you for this. Took me 3 days to figure out why my textures weren't working!!

    if (renderTarget != nullptr && renderTarget->NeedsCubeRendering()) {
        HelperCubeRender(mesh, instances, shader);
    } else {
        glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(mainCamera->GetViewMatrix()));
        mesh->Render(instances);
    }
}

void ControlledScene3D::RenderMeshCustomMaterial(Mesh *mesh, Material material, const glm::mat4 &modelMatrix)
{
    if (!mesh || !material.shader || !material.shader->GetProgramID())
        return;

    material.Use();
    RenderMesh(mesh, material.shader, material.instances, modelMatrix);
}

void ControlledScene3D::HelperCubeRender(Mesh *mesh, int instances, Shader *shader)
{
    GLuint loc_view_matrix = glGetUniformLocation(shader->program, "WIST_VIEW_MATRIX");
    GLuint loc_cube_face = glGetUniformLocation(shader->program, "WIST_CUBE_FACE");

    glm::vec3 cameraPos = mainCamera->GetPosition();
    glm::vec3 cameraForward = mainCamera->GetForward();
    glm::vec3 cameraUp = mainCamera->GetUp();
    glm::vec3 cameraRight = mainCamera->GetRight();
    glm::mat4 viewMatrices[6] = {
        glm::lookAt(cameraPos, cameraPos + cameraRight,   -cameraUp),   // +X
        glm::lookAt(cameraPos, cameraPos - cameraRight,   -cameraUp),   // -X
        glm::lookAt(cameraPos, cameraPos + cameraUp,  cameraForward),   // +Y
        glm::lookAt(cameraPos, cameraPos - cameraUp, -cameraForward),   // -Y
        glm::lookAt(cameraPos, cameraPos + cameraForward, -cameraUp),   // +Z
        glm::lookAt(cameraPos, cameraPos - cameraForward, -cameraUp),   // -Z
    };
    FrameBuffer::Shape shape = renderTarget->GetShape();
    for (unsigned char face = 0; face < 6; ++face) {
        glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrices[face]));
        glUniform1i(loc_cube_face, face);
        for (auto att : shape.colorAttachments) {
            if (shape.colorDescriptors[att].direction != FrameBuffer::CUBE)
                continue;
            renderTarget->AttachCubemapFace(att, face);
        }
        if (shape.depthDescriptor.direction == FrameBuffer::CUBE) {
            renderTarget->AttachDepthCubemapFace(face);
        }
        mesh->Render(instances);
    }
}

void ControlledScene3D::CheckCollisions()
{
    std::unordered_set<std::pair<GameObject *, GameObject *>, pair_hash> pairs;
    for (int layer1 = 0; layer1 < 32; ++layer1) {
        for (int layer2 = layer1; layer2 < 32; ++layer2) {
            if ((collisionMasks[layer1] & (1 << layer2)) == 0)
                continue;
            
            for (auto gameObject1 : layers[layer1]) {
                for (auto gameObject2 : layers[layer2]) {
                    if (gameObject1 == gameObject2 ||
                        pairs.find({gameObject1, gameObject2}) != pairs.end()) continue;
                    
                    std::unique_ptr<CollisionEvent> event1, event2;
                    pairs.insert(std::make_pair(gameObject1, gameObject2));
                    if (gameObject1->Collides(gameObject2, event1, event2)) {
                        event1->Dispatch(gameObject1);
                        event2->Dispatch(gameObject2);
                    }
                }
            }
        }
    }
}

void ControlledScene3D::InitGBuffer()
{
    for (auto camera : cameras) {
        int width, height;
        FrameBuffer::Direction direction;

        if (camera->renderTarget != nullptr) {
            FrameBuffer::Shape shape = camera->renderTarget->GetShape();
            width = shape.width;
            height = shape.height;
            direction = shape.colorDescriptors[0].direction;
        } else {
            width = drawAreaWidth;
            height = drawAreaHeight;
            direction = FrameBuffer::TEX2D;
        }
        delete camera->gBuffer;
        camera->gBuffer = new FrameBuffer(width, height);
        camera->gBuffer->SetColorTexture(0, Texture::RGBA32F, direction);  // color texture
        camera->gBuffer->SetColorTexture(1, Texture::RGBA32F, direction);  // normal texture 
        camera->gBuffer->SetColorTexture(2, Texture::RGBA32F, direction);  // world position texture
        camera->gBuffer->SetColorTexture(3, Texture::RGBA32F, direction);  // light accumulation texture
        camera->gBuffer->SetDepthTexture(true, Texture::DEPTH32F, direction);
    }
}

Shader *ControlledScene3D::HelperDefferedShader(std::string shaderName)
{
    if (gBuffer != nullptr && gBuffer->NeedsCubeRendering())
        shaderName += "/Cube";
    return Assets::shaders[shaderName];
}

void ControlledScene3D::AccumulateLight(Light *light, bool update)
{
    bool cubeRender = gBuffer->NeedsCubeRendering();
    Shader *shader = HelperDefferedShader("Deffered/LightAccumulate");
    Material &material = light->material;
    material.shader = shader;
    material.SetTexture("TEXTURE_NORMAL", gBuffer->GetColorTexture(1));
    material.SetTexture("TEXTURE_WORLD_POSITION", gBuffer->GetColorTexture(2));

    if (update) {
        float deltaTime = light->useUnscaledTime ? unscaledDeltaTime : this->deltaTime;
        light->Tick(deltaTime);
    }

    light->Use();
    material.Use();
    RenderMesh(Assets::meshes["Default/Sphere"], shader, 1, light->ObjectToWorldMatrix());
}

void ControlledScene3D::OnInputUpdate(float deltaTime, int mods)
{
    OnInputUpdate(mods);

    if (!window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) return;
    if (!controlsEnabled) return;

    float disp = deltaTime * controls.speed;
    glm::vec3 forward = mainCamera->GetForward();
    forward = glm::normalize(glm::vec3(forward.x, 0, forward.z));

    if (window->KeyHold(controls.forward))   mainCamera->Translate(forward * disp);
    if (window->KeyHold(controls.backward))  mainCamera->Translate(-forward * disp);
    if (window->KeyHold(controls.left))      mainCamera->Translate(mainCamera->GetRight() * disp);
    if (window->KeyHold(controls.right))     mainCamera->Translate(-mainCamera->GetRight() * disp);
    if (window->KeyHold(controls.up))        mainCamera->Translate(glm::vec3_up * disp);
    if (window->KeyHold(controls.down))      mainCamera->Translate(glm::vec3_down * disp);
}

void ControlledScene3D::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    OnMoveMouse(mouseX, mouseY, deltaX, deltaY);

    if (!mouseControlsEnabled) return;

    if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) {
        mainCamera->Rotate(glm::angleAxis(0.005f * -deltaX, glm::vec3_up));
        mainCamera->Rotate(glm::angleAxis(0.005f * deltaY, mainCamera->GetRight()));
    }
}

void ControlledScene3D::EnableControls(Controls controls) { 
    this->controlsEnabled = true;
    this->controls = controls; 
}
void ControlledScene3D::DisableControls() { 
    this->controlsEnabled = false;
    this->controls = { 0 }; 
}
void ControlledScene3D::EnableMouseControls() { mouseControlsEnabled = true; }
void ControlledScene3D::DisableMouseControls() { mouseControlsEnabled = false; }

void ControlledScene3D::ResizeDrawArea()
{
    const float windowAspectRatio = windowResolution.x / (float)windowResolution.y;
    if (windowAspectRatio > aspectRatio) {
        const int w = (int)(windowResolution.y * aspectRatio);
        drawAreaX = (windowResolution.x - w) / 2;
        drawAreaY = 0;
        drawAreaWidth = w;
        drawAreaHeight = windowResolution.y;
    }
    else {
        const int h = (int)(windowResolution.x / aspectRatio);
        drawAreaX = 0;
        drawAreaY = (windowResolution.y - h) / 2;
        drawAreaWidth = windowResolution.x;
        drawAreaHeight = h;
    }

    InitGBuffer();
}

void ControlledScene3D::OnWindowResize(int width, int height)
{
    windowResolution = window->GetResolution();
    if (windowResolution.x == 0 || windowResolution.y == 0)
        return;
    ResizeDrawArea();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    OnResizeWindow();
}
