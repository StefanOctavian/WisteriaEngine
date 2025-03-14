#version 330

in vec3 frag_color;
in vec3 frag_normal;
in vec3 frag_world_pos;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_world_pos;

void main()
{
    out_color = vec4(frag_normal, 1);
    out_normal = vec4(frag_normal, 1);
    out_world_pos = vec4(frag_world_pos, 1);
}
