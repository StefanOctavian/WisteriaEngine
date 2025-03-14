#include "mountain.h"

using namespace engine;
using namespace mountain;

#define GROUND_W 40
#define GROUND_L 40
#define GROUND_NUM_INSTANCES 100

#define LAYER_REFLECTIVE ((unsigned)1)

Mountain::Mountain() : ControlledScene3D() {}
Mountain::~Mountain() {}

void Mountain::Initialize() {
    MeshPlusPlus *fireflyMesh = new MeshPlusPlus("firefly", Assets::meshes["Default/Sphere"]);
    for (auto &vertex : fireflyMesh->vertices) {
        vertex.color = glm::vec3(0, 1, 1);
    }
    Assets::meshes["firefly"] = fireflyMesh;

    const std::string shaders = PATH_JOIN(SOURCE_PATH::MAIN, "mountain", "shaders");
    Assets::AddPath("Mountain.GS", PATH_JOIN(shaders, "Mountain.GS.glsl"));
    Assets::AddPath("Mountain.FS", PATH_JOIN(shaders, "Mountain.FS.glsl"));
    Assets::AddPath("Lake.FS", PATH_JOIN(shaders, "Lake.FS.glsl"));
    Assets::AddPath("Drops.FS", PATH_JOIN(shaders, "Drops.FS.glsl"));
    Assets::LoadShader("Mountain", "Default.Model.VS", "Mountain.GS", "Mountain.FS");
    Assets::LoadShader("Mountain/Normal", "Default.Model.VS", "Mountain.GS", "Default.NormalColor.FS");
    Assets::LoadShader("Lake", "Default.VS", "Lake.FS");

    const std::string textures = PATH_JOIN(SOURCE_PATH::MAIN, "mountain", "textures");
    const std::string cubemap  = PATH_JOIN(textures, "cubemap_night");
    Assets::LoadTexture2D("Mountain", textures, "mountain.jpeg");
    Assets::LoadTexture2D("PerlinNoise", textures, "perlinNoise2.png");
    Assets::LoadTexture2D("Raindrop", textures, "rain.png");
    Assets::LoadTexture2D("Foam", textures, "foam.png");
    Assets::LoadCubemapTexture("Skybox", cubemap, "pos_x.png", "neg_x.png", "pos_y.png", 
                                                  "neg_y.png", "pos_z.png", "neg_z.png");

    CreateTerrain();
    CreateLake();

    EnableControls(basicControls);
    EnableMouseControls();
    SetupScene();
}

void Mountain::CreateTerrain() {
    MeshPlusPlus *ground = new MeshPlusPlus("ground");
    ground->vertices = { 
        VertexFormat(glm::vec3(0), glm::vec3(1, 1, 1), glm::vec3_up),
    };
    ground->indices = { 0 };
    ground->SetDrawMode(GL_POINTS);
    ground->InitFromData(ground->vertices, ground->indices);
    Assets::meshes["ground"] = ground;
}

void Mountain::CreateLake() {
    MeshPlusPlus *lake = new MeshPlusPlus("lake");
    lake->vertices = {
        VertexFormat(glm::vec3(-GROUND_W / 4, 0,  GROUND_L / 4), glm::vec3(1, 0, 0), glm::vec3_up),
        VertexFormat(glm::vec3(-GROUND_W / 4, 0, -GROUND_L / 4), glm::vec3(0, 0, 1), glm::vec3_up),
        VertexFormat(glm::vec3( GROUND_W / 4, 0,  GROUND_L / 4), glm::vec3(0, 0, 1), glm::vec3_up),
        VertexFormat(glm::vec3( GROUND_W / 4, 0, -GROUND_L / 4), glm::vec3(0, 1, 0), glm::vec3_up)
    };
    lake->indices = { 0, 1, 2, 3 };
    lake->SetDrawMode(GL_TRIANGLE_STRIP);
    lake->InitFromData(lake->vertices, lake->indices);
    Assets::meshes["lake"] = lake;
}

void Mountain::CreateWaterfall()
{
    // waterfall
    glm::vec3 controlPoints[] = { glm::vec3(0, 14, 20), glm::vec3(0, 0, 5), glm::vec3(0, 0, 0) };
    ParticleSystem *waterfall = new ParticleSystem();
    waterfall->name = "Waterfall";
    waterfall->SetPosition(glm::vec3(0, 14, 20));
    waterfall->maxParticles = 50000;
    waterfall->particleSize = 0.2f;
    waterfall->duration = 10;
    waterfall->delay = 0.0;
    waterfall->initLifetime = 5;
    waterfall->initVelocity = -2.0f / 10.0f * controlPoints[0] + 2.0f / 10.0f * controlPoints[1];
    waterfall->acceleration = 2.0f / 100.0f * (controlPoints[0] - 2.0f * controlPoints[1] + controlPoints[2]);
    waterfall->boxSize = glm::vec3(1, 1, 1);
    waterfall->material.texture = Assets::textures["Raindrop"];
    waterfall->SwitchFragmentShader("Drops.FS");
    AddToLayer(waterfall, LAYER_REFLECTIVE);

    this->waterfall = waterfall;
    // (1-t)^2 A + 2(1-t)tB + t^2 C
    // -2(1-t)A + 2(1-2t)B + 2tC  => v0 = -2A + 2B
    // a = 2A - 4B + 2C

    // foam
    ParticleSystem *foam = new ParticleSystem();
    foam->name = "Foam";
    foam->SetPosition(glm::vec3(0, 5, 9));
    foam->maxParticles = 5000;
    foam->particleSize = 0.1f;
    foam->duration = 4;
    foam->delay = 10;
    foam->initLifetime = 4;
    foam->initVelocity = glm::vec3(0, 0.5, 0);
    foam->acceleration = glm::vec3(0, 0, 0);
    foam->boxSize = glm::vec3(2, 0.8, 1.5);
    foam->material.texture = Assets::textures["Foam"];
    foam->SwitchFragmentShader("Drops.FS");

    AddToLayer(foam, LAYER_REFLECTIVE);
    waterfall->AddChild(foam);
}

float rand01() {
    return (float)rand() / RAND_MAX;
}

void Mountain::AddLights()
{
    center = new GameObject(Assets::meshes["Default/Sphere"], glm::vec3(0, 15, 0));
    center->angularVelocity = glm::vec3(0, 0.25, 0);

    defferedRendering = true;

    for (int i = 0; i < 5; ++i) {
        float radius = -15 + 5 * (rand01() * 2 - 1);
        float angle = (float)(rand01() * 2 * M_PI);
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float y = 20 + 5 * (rand01() * 2 - 1);
        float range = 15 - 3 * rand01();
        glm::vec3 color = glm::vec3(rand01(), rand01(), rand01());
        Light *light = new PointLight(glm::vec3(x, y, z), color, range);
        
        GameObject *firefly = new GameObject(Assets::meshes["firefly"], glm::vec3(x, y, z));
        firefly->material.shader = Assets::shaders["VertexColor"];
        light->AddChild(firefly);
        center->AddChild(light);
    }
}

void Mountain::SetupScene() {
    mainCamera->SetPosition(glm::vec3(0, 30, -30));
    mainCamera->SetRotation(glm::vec3(0.7, 0, 0));
    mainCamera->SetPerspective(60, 1280 / 720.0f, 0.01f, 1500.0f);
    mainCamera->name = "Main Camera";

    fbo = new FrameBuffer(1024, 1024);
    fbo->SetColorTexture(0, Texture::RGBA32F, FrameBuffer::CUBE);
    fbo->SetDepthTexture(true, FrameBuffer::CUBE);

    Camera *lakeCamera = new Camera(glm::vec3(0, 5.5, 0), glm::vec3_forward, glm::vec3_up);
    lakeCamera->name = "Lake Camera";
    lakeCamera->SetPerspective(90, 1, 0.1f, 1000.0f);
    lakeCamera->renderTarget = fbo;
    lakeCamera->cullingMask = ~(1U << LAYER_REFLECTIVE);  // ignore reflective layer
    cameras.push_back(lakeCamera);

    // ground
    ground = new GameObject(Assets::meshes["ground"], glm::vec3(0));
    ground->name = "ground";
    ground->material.shader = Assets::shaders["Mountain"];
    ground->material.instances = GROUND_NUM_INSTANCES;
    ground->material.SetInt("NUM_INSTANCES", GROUND_NUM_INSTANCES);
    ground->material.SetFloat("WIDTH", GROUND_W);
    ground->material.SetFloat("LENGTH", GROUND_L);
    ground->material.SetTexture("PERLIN_NOISE", Assets::textures["PerlinNoise"]);
    ground->material.texture = Assets::textures["Mountain"];

    // skybox
    GameObject *skybox = new GameObject(Assets::meshes["Default/Cube"], glm::vec3(0, 50, 0), glm::vec3(1200));
    skybox->material.shader = Assets::shaders["Skybox"];
    skybox->material.SetTexture("TEXTURE_CUBEMAP", Assets::textures["Skybox"]);

    // lake
    lake = new GameObject(Assets::meshes["lake"], glm::vec3(0, 5.5, 0));
    lake->material.shader = Assets::shaders["Lake"];
    lake->material.SetTexture("TEXTURE_CUBEMAP", fbo->GetColorTexture(0));
    AddToLayer(lake, LAYER_REFLECTIVE);

    CreateWaterfall();
    AddLights();

    AddToScene(ground);
    AddToScene(skybox);
    AddToScene(lake);
    AddToScene(center);
    AddToScene(waterfall);
}

void Mountain::Tick() 
{

}

void Mountain::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_P) {
        timeScale = timeScale == 0 ? 1.0f : 0.0f;
    }

    if (key == GLFW_KEY_G) {
        if (ground->material.texture) {
            ground->material.shader = Assets::shaders["Mountain/Normal"];
            ground->material.texture = nullptr;
        } else {
            ground->material.shader = Assets::shaders["Mountain"];
            ground->material.texture = Assets::textures["Mountain"];
        } 
    }
}

void Mountain::OnResizeWindow() {
    lake->material.SetTexture("TEXTURE_CUBEMAP", fbo->GetColorTexture(0));
}

