#include "particlesystem.h"
#include "meshplusplus.h"
#include "assets.h"

using namespace engine;

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::~ParticleSystem()
{
    glDeleteBuffers(1, &ssbo);
}

void ParticleSystem::Initialize()
{
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(WistParticle), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    mesh = new MeshPlusPlus("__particle_system");
    mesh->vertices = {
        VertexFormat(glm::vec3(0), glm::vec3(1, 1, 1), glm::vec3_up),
    };
    mesh->indices = { 0 };
    mesh->SetDrawMode(GL_POINTS);
    mesh->InitFromData(mesh->vertices, mesh->indices);
    material.instances = 0;

    material.SetVec3("WIST_PARTICLE_SYSTEM_POSITION", position);
    material.SetFloat("WIST_PARTICLE_SIZE", particleSize);

    if (!material.shader)
        material.shader = Assets::shaders["Particles"];
}

void ParticleSystem::Tick(float deltaTime)
{
    GameObject::Tick(deltaTime);
    if (activeParticles < maxParticles) {
        Emit((int)(maxParticles / duration * deltaTime + 0.5f));
    }

    material.SetVec3("WIST_PARTICLE_SYSTEM_POSITION", position);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    // the unbinding is done after rendering by ControlledScene3D
}

void ParticleSystem::Emit(int maxCount)
{
    maxCount = glm::min(maxCount, maxParticles - activeParticles);
    if (maxCount <= 0)
        return;  // no more particles to emit

    std::vector<WistParticle> particles(maxCount);
    for (int i = 0; i < maxCount; ++i) {
        particles[i].position = glm::vec3(rand11() * boxSize.x, rand11() * boxSize.y, rand11() * boxSize.z);
        particles[i].velocity = initVelocity;
        particles[i].acceleration = acceleration;
        particles[i].init_position = particles[i].position;
        particles[i].init_velocity = initVelocity;
        particles[i].delay = delay;
        particles[i].init_delay = delay;
        particles[i].lifetime = initLifetime;
        particles[i].init_lifetime = initLifetime;
    }

    size_t size = maxCount * sizeof(WistParticle);
    size_t base = activeParticles * sizeof(WistParticle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, base, size, particles.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    activeParticles += maxCount;
    material.instances = activeParticles;
}

void ParticleSystem::SwitchFragmentShader(std::string fragShaderName)
{
    Shader *shader = Assets::CreateShader("Particles.VS", "Particles.GS", fragShaderName);
    material.shader = shader;
}