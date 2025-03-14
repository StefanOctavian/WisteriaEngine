#version 330
// FOR SOME REASON, THIS SHADER IS A HUGE BOTTLENECK WHEN USING TEXTURES
// AVOID USING IT IF POSSIBLE AND CREATE A CUSTOM SHADER INSTEAD

in vec3 frag_world_pos;
in vec3 frag_normal;
in vec3 frag_color;
in vec2 frag_tex_coord;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_world_position;

uniform sampler2D WIST_TEXTURE;
uniform bool WIST_USE_TEXTURE;

void main()
{
    out_color = WIST_USE_TEXTURE ? texture(WIST_TEXTURE, frag_tex_coord) : vec4(frag_color, 1);
    out_normal = vec4(normalize(frag_normal), 1);
    out_world_position = vec4(frag_world_pos, 1);
}