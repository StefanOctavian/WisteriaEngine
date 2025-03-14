#version 430

struct Particle {
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    vec3 init_position;
    vec3 init_velocity;
    float delay;
    float init_delay;
    float lifetime;
    float init_lifetime;
};

layout(std430, binding = 0) buffer particles {
    Particle data[];
};

uniform mat4 WIST_MODEL_MATRIX;
uniform vec3 WIST_PARTICLE_SYSTEM_POSITION;
uniform float WIST_DELTA_TIME;

void main()
{
    vec3 position = data[gl_InstanceID].position;
    vec3 velocity = data[gl_InstanceID].velocity;
    vec3 acceleration = data[gl_InstanceID].acceleration;
    float delay = data[gl_InstanceID].delay;
    float lifetime = data[gl_InstanceID].lifetime;

    delay -= WIST_DELTA_TIME;
    if (delay > 0) {
        data[gl_InstanceID].delay = delay;
        gl_Position = WIST_MODEL_MATRIX * vec4(WIST_PARTICLE_SYSTEM_POSITION, 1);
        return;
    }

    lifetime -= WIST_DELTA_TIME;
    if (lifetime <= 0) {
        data[gl_InstanceID].position = data[gl_InstanceID].init_position;
        data[gl_InstanceID].velocity = data[gl_InstanceID].init_velocity;
        data[gl_InstanceID].delay = 0;
        data[gl_InstanceID].lifetime = data[gl_InstanceID].init_lifetime;
        gl_Position = WIST_MODEL_MATRIX * vec4(WIST_PARTICLE_SYSTEM_POSITION, 1);
        return;
    }
    
    position += velocity * WIST_DELTA_TIME;
    velocity += acceleration * WIST_DELTA_TIME;

    data[gl_InstanceID].position = position;
    data[gl_InstanceID].velocity = velocity;
    data[gl_InstanceID].delay = delay;
    data[gl_InstanceID].lifetime = lifetime;

    gl_Position = WIST_MODEL_MATRIX * vec4(position, 1);
}
