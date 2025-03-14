#version 330
#define WIST_USE_TEXTURE

in vec3 frag_world_pos;
in vec3 frag_normal;
in vec2 frag_tex_coord;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_world_position;

uniform sampler2D WIST_TEXTURE;

void main()
{
    #ifdef WIST_USE_TEXTURE 
    out_color = texture(WIST_TEXTURE, frag_tex_coord);
    #else
    out_color = vec4(1, 1, 1, 1);
    #endif
    out_normal = vec4(frag_normal, 1);
    out_world_position = vec4(frag_world_pos, 1);
}