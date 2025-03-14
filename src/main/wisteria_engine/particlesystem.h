#pragma once
#include "utils/glm_utils.h"
#include "gameobject3d.h"

namespace engine 
{
    struct Particle
    {
        Particle() {}
        ~Particle() {}
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        float initialLifetime;
        float remainingLifetime;
    };

    // disclaimer: this particle system is rather simple because I don't have
    // much time left for the research and implementation. It will be improved later.
    class ParticleSystem : public GameObject
    {
    public:
        ParticleSystem();
        virtual ~ParticleSystem();

        void Initialize() override;
        void Tick(float deltaTime) override;

        int maxParticles = 100;
        float duration = 1;
        float delay = 0;
        float initLifetime = 1;
        glm::vec3 initVelocity = glm::vec3(0);
        glm::vec3 acceleration = glm::vec3(0);
        float particleSize = 0.5;

        // box emitter - will change to a more general emitter later
        glm::vec3 boxSize = glm::vec3(1);

        void SwitchFragmentShader(std::string shaderName);

    private:
        GLuint ssbo = 0;
        int activeParticles = 0;
        inline float rand11() { return 2 * (float)rand() / RAND_MAX - 1; }

    protected:
        void Emit(int maxCount);

        // the particle struct that will be passed to the shader
        // the naming of the fields matches the ones in the shader
        struct WistParticle {
            glm::vec3 position;
            float _padding1;
            glm::vec3 velocity;
            float _padding2;
            glm::vec3 acceleration;
            float _padding3;
            glm::vec3 init_position;
            float _padding4;
            glm::vec3 init_velocity;
            float _padding5;
            float delay;
            float init_delay;
            float lifetime;
            float init_lifetime;
        };
    };
};