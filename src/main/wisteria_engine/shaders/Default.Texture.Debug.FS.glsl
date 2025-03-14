#version 330

in vec3 frag_normal;
in vec2 frag_tex_coord;

uniform sampler2D WIST_TEXTURE;

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(1, 0, 0, 1);
}
