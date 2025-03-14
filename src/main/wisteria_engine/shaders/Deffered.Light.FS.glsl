#version 330

uniform sampler2D TEXTURE_WORLD_POSITION;
uniform sampler2D TEXTURE_NORMAL;

uniform vec3 LIGHT_POSITION;
uniform vec3 LIGHT_COLOR;
uniform float LIGHT_RANGE;

uniform vec3 WIST_MATERIAL_DIFFUSE;
uniform vec3 WIST_MATERIAL_SPECULAR;
uniform float WIST_MATERIAL_SHININESS;

uniform ivec2 WIST_RESOLUTION;
uniform vec4 WIST_EYE_POSITION;

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

void main()
{
    vec2 tex_coord = gl_FragCoord.xy / WIST_RESOLUTION;
    vec3 world_pos = texture(TEXTURE_WORLD_POSITION, tex_coord).xyz;
    vec3 world_normal = texture(TEXTURE_NORMAL, tex_coord).xyz;

    out_lights = vec4(Phong_light(world_pos, world_normal), 1);
}





