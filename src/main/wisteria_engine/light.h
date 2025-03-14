#pragma once
#include "material.h"
#include "gameobject3d.h"

namespace engine
{
    class GameObject;
    class Light : public GameObject
    {
    public:
        glm::vec3 color;
        
        Light(glm::vec3 color, glm::vec3 position, glm::vec3 scale): 
            GameObject(position, scale), color(color) {}
        virtual ~Light() {}

        inline static glm::vec3 ambientLight = glm::vec3(0.7f, 0.7f, 0.7f);

    protected:
        friend class ControlledScene3D;
        virtual void Use() {
            material.SetVec3("LIGHT_POSITION", GetPosition());
            material.SetVec3("LIGHT_COLOR", color);
        }
    };

    class PointLight : public Light
    {
    public:
        float range;

        PointLight(glm::vec3 position, glm::vec3 color, float range): 
            Light(color, position, glm::vec3(1, 1, 1) * (2 * range)), range(range) {}
        virtual ~PointLight() {}

    protected:
        virtual void Use() override {
            Light::Use();
            material.SetFloat("LIGHT_RANGE", range);
        }
    };
}