#version 330

uniform samplerCube TEXTURE_WORLD_POSITION;
uniform samplerCube TEXTURE_NORMAL;

uniform vec3 LIGHT_POSITION;
uniform vec3 LIGHT_COLOR;
uniform float LIGHT_RANGE;

uniform vec3 WIST_MATERIAL_DIFFUSE;
uniform vec3 WIST_MATERIAL_SPECULAR;
uniform float WIST_MATERIAL_SHININESS;

uniform ivec2 WIST_RESOLUTION;
uniform vec4 WIST_EYE_POSITION;
uniform int WIST_CUBE_FACE;

layout(location = 3) out vec4 out_lights;

vec3 Phong_light(vec3 world_pos, vec3 world_normal)
{
    vec3 L = normalize(LIGHT_POSITION - world_pos);

    float dist = distance(LIGHT_POSITION, world_pos);

    // Ignore fragments outside of the
    // light influence zone (range)
    if (dist > LIGHT_RANGE)
        return vec3(0);

    float attenuation = pow(LIGHT_RANGE - dist, 2);

    float dot_specular = dot(world_normal, L);
    vec3 specular = vec3(0);
    if (dot_specular > 0) {
        vec3 V = normalize(WIST_EYE_POSITION.xyz - world_pos);
        vec3 H = normalize(L + V);
        specular = WIST_MATERIAL_SPECULAR * pow(max(dot(world_normal, H), 0), WIST_MATERIAL_SHININESS);
    }

    vec3 diffuse = WIST_MATERIAL_DIFFUSE * max(dot_specular, 0);

    return attenuation * (diffuse + specular) * LIGHT_COLOR;
}

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
    vec2 tex_coord = gl_FragCoord.xy / WIST_RESOLUTION;
    vec3 world_pos = texture(TEXTURE_WORLD_POSITION, uvToDir(tex_coord, WIST_CUBE_FACE)).xyz;
    vec3 world_normal = texture(TEXTURE_NORMAL, uvToDir(tex_coord, WIST_CUBE_FACE)).xyz;

    out_lights = vec4(Phong_light(world_pos, world_normal), 1);
    // out_lights = vec4(world_pos, 1);
}