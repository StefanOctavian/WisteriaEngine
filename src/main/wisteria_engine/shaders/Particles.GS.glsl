#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 WIST_VIEW_MATRIX;
uniform mat4 WIST_PROJECTION_MATRIX;
uniform vec4 WIST_EYE_POSITION;
uniform float WIST_PARTICLE_SIZE;

out vec2 frag_tex_coord;

vec3 v_pos = gl_in[0].gl_Position.xyz;
vec3 forward = normalize(WIST_EYE_POSITION.xyz - v_pos);
vec3 right = normalize(cross(forward, vec3(0, 1, 0)));
vec3 up = normalize(cross(forward, right));


void EmitPoint(vec2 offset)
{
    vec3 pos = right * offset.x + up * offset.y + v_pos;
    gl_Position = WIST_PROJECTION_MATRIX * WIST_VIEW_MATRIX * vec4(pos, 1.0);
    EmitVertex();
}


void main()
{
    float ds = WIST_PARTICLE_SIZE / 2.0;

    frag_tex_coord = vec2(0, 0);
    EmitPoint(vec2(-ds, -ds));

    frag_tex_coord = vec2(1, 0);
    EmitPoint(vec2(ds, -ds));

    frag_tex_coord = vec2(0, 1);
    EmitPoint(vec2(-ds, ds));

    frag_tex_coord = vec2(1, 1);
    EmitPoint(vec2(ds, ds));

    EndPrimitive();
}
