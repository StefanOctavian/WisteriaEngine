#version 330

in vec3 frag_normal;
in vec2 frag_tex_coord;

uniform sampler2D WIST_TEXTURE;

layout(location = 0) out vec4 out_color;


void main()
{
    out_color = texture(WIST_TEXTURE, frag_tex_coord);
    if (out_color.a < 0.9)
    {
        discard;
    }
}
