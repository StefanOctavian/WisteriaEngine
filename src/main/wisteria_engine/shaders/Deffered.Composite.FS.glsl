#version 330

in vec2 frag_tex_coord;

uniform vec3 WIST_AMBIENT_LIGHT;
uniform sampler2D TEXTURE_COLOR;
uniform sampler2D TEXTURE_LIGHT;

layout(location = 0) out vec4 out_color;

void main()
{
    vec3 ambient = WIST_AMBIENT_LIGHT;
    vec3 color = texture(TEXTURE_COLOR, frag_tex_coord).xyz;
    vec3 light = texture(TEXTURE_LIGHT, frag_tex_coord).xyz;
    out_color = vec4((ambient + light) * color, 1);
}