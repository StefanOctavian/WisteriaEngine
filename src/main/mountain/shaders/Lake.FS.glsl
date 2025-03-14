#version 330

in vec3 frag_world_pos;
in vec3 frag_normal;

uniform samplerCube TEXTURE_CUBEMAP;
uniform vec4 WIST_EYE_POSITION;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_world_pos;

void main() {
    vec3 incident_ray = frag_world_pos - WIST_EYE_POSITION.xyz;
    out_color = texture(TEXTURE_CUBEMAP, reflect(incident_ray, frag_normal));
    out_normal = vec4(frag_normal, 1);
    out_world_pos = vec4(frag_world_pos, 1);
}
