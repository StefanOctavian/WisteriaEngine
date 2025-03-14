#version 330

in vec2 frag_tex_coord;

out vec4 out_color;

uniform sampler2D WIST_TEXTURE;

void main()
{
    vec4 color = texture(WIST_TEXTURE, frag_tex_coord);
    if (length(color.rgb) < 0.1) {
        discard;
    }
    out_color = vec4(color.rgb, 1);
}
