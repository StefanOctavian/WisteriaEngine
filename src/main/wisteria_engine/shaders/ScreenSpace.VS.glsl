#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
layout(location = 3) in vec3 v_color;

out vec3 frag_color;
out vec2 frag_tex_coord;

void main()
{
    frag_color = v_color;
    frag_tex_coord = v_texture_coord;

    gl_Position = vec4(v_position, 1.0);
}
