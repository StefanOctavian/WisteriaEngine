#pragma once
#include <set>
#include <unordered_set>
#include <unordered_map>
#include "camera.h"
#include "meshplusplus.h"
#include "light.h"

#include "components/simple_scene.h"

namespace engine
{
    class GameObject;
    class ControlledScene3D : public gfxc::SimpleScene
    {
    public:
        ControlledScene3D();
        ~ControlledScene3D();
        void Init() override;

        void AddToScene(GameObject *gameObject);
        void Destroy(GameObject *gameObject);
        void AddToLayer(GameObject *gameObject, int layer);
        void RemoveFromLayer(GameObject *gameObject, int layer);

    protected:
        virtual void Initialize() {}; 
        virtual void Tick() {};
        virtual void OnResizeWindow() {};
        virtual void OnInputUpdate(int mods) {};
        virtual void OnMoveMouse(int mouseX, int mouseY, int deltaX, int deltaY) {};
        
        struct Controls {
            int forward, backward, left, right, up, down;
            float speed;
        } basicControls = {
            GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_Q, 6
        };
        void EnableControls(Controls controls);
        void DisableControls();
        void EnableMouseControls();
        void DisableMouseControls();
        void SetAspectRatio(float aspectRatio) { 
            this->aspectRatio = aspectRatio; 
            ResizeDrawArea();
        }

        // glm::vec2 ScreenCoordsToLogicCoords(int screenX, int screenY);

    private:
        void FrameStart() override;
        void InitMeshes();
        void InitShaders();
        void Update(float deltaTimeSeconds) override;
        void ForwardRenderScene(bool update = false);
        void DefferedRenderScene(bool update = false);
        void CheckCollisions();
        void OnInputUpdate(float deltaTime, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        // void FrameEnd() override;

        void ResizeDrawArea();
        void OnWindowResize(int width, int height) override;
        
        inline Shader *HelperDefferedShader(std::string shaderName);
        void HelperCubeRender(Mesh *mesh, int instances, Shader *shader);
        void RenderMesh(Mesh *mesh, Shader *shader, int instances, const glm::mat4 &modelMatrix);
        void RenderMeshCustomMaterial(Mesh *mesh, Material material, const glm::mat4 &modelMatrix);
        void DrawGameObject(GameObject *gameObject, bool update = false);

        void InitGBuffer();
        void AccumulateLight(Light *light, bool update = false);

    protected:
        glm::vec4 clearColor = glm::vec4(0, 0, 0, 1);
        std::unordered_set<GameObject *> gameObjects;
        float deltaTime;
        float unscaledDeltaTime;
        float timeScale = 1;

        std::vector<Camera *> cameras;
        Camera *mainCamera;

        std::vector<Light *> lights;
        bool defferedRendering = false;

        std::vector<int> collisionMasks;

        FrameBuffer *renderTarget = nullptr;  // current render target
        FrameBuffer *gBuffer = nullptr;  // current G-Buffer

    private:
        std::unordered_set<GameObject *> toDestroy;
        std::vector<std::unordered_set<GameObject *>> layers;

        glm::ivec2 windowResolution;
        float aspectRatio = 16.0f / 9.0f;
        int drawAreaX, drawAreaY, drawAreaWidth, drawAreaHeight;

        Controls controls;
        bool controlsEnabled = false;
        bool mouseControlsEnabled = false;
    };
    void checkFBStatus();
} // namespace engine