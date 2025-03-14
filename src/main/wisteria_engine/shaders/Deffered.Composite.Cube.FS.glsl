#version 330

in vec2 frag_tex_coord;

uniform vec3 WIST_AMBIENT_LIGHT;
uniform samplerCube TEXTURE_COLOR;
uniform samplerCube TEXTURE_LIGHT;
uniform int WIST_CUBE_FACE;

layout(location = 0) out vec4 out_color;

vec3 uvToDir(vec2 uv, int face)
{
    uv = uv * 2 - 1.0;
    uv.y *= -1.0;

    if (face == 0) return vec3(1.0, uv.y, -uv.x);   // +X
    if (face == 1) return vec3(-1.0, uv.y, uv.x);   // -X
    if (face == 2) return vec3(uv.x, 1.0, -uv.y);   // +Y
    if (face == 3) return vec3(uv.x, -1.0, uv.y);   // -Y
    if (face == 4) return vec3(uv.x, uv.y, 1.0);    // +Z
    if (face == 5) return vec3(-uv.x, uv.y, -1.0);  // -Z
}

void main()
{
    vec3 ambient = WIST_AMBIENT_LIGHT;
    vec3 color = texture(TEXTURE_COLOR, uvToDir(frag_tex_coord, WIST_CUBE_FACE)).xyz;
    vec3 light = texture(TEXTURE_LIGHT, uvToDir(frag_tex_coord, WIST_CUBE_FACE)).xyz;
    out_color = vec4((ambient + light) * color, 1.0);
}