#pragma once

#include "../wisteria_engine/controlledscene3d.h"
#include "../wisteria_engine/assets.h"
#include "../wisteria_engine/framebuffer.h"
#include "../wisteria_engine/particlesystem.h"

using namespace engine;

namespace mountain
{
    class Mountain : public ControlledScene3D
    {
    public:
        Mountain();
        ~Mountain();

    private:
        void Initialize() override;
        void Tick() override;
        void OnKeyPress(int key, int mods) override;
        void OnResizeWindow() override;

        void SetupScene();
        void CreateTerrain();
        void CreateLake();
        void CreateWaterfall();
        void AddLights();

        FrameBuffer *fbo;
        int frame = 0;
        GameObject *ground;
        GameObject *lake;
        GameObject *waterfall;
        GameObject *center;
        GameObject *debugCube;
    };
}

