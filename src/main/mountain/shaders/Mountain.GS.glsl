#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 256) out;

uniform mat4 WIST_VIEW_MATRIX;
uniform mat4 WIST_PROJECTION_MATRIX;

in vec3 gs_color[];
in int gs_instance_id[];

out vec3 frag_color;
out vec3 frag_normal;
out vec2 frag_tex_coord;
out vec3 frag_world_pos;

uniform float WIDTH;
uniform float LENGTH;
uniform int NUM_INSTANCES;
uniform sampler2D PERLIN_NOISE;

// uniform vec2 CONTROL_POINTS[3];
// uniform float CANAL_RADIUS;
vec3 CONTROL_POINTS[3] = vec3[3](
    vec3(0, 0, 0), vec3(0, 0, 20), vec3(0, 65, 50)
);
float CANAL_RADIUS = 3;

float du;
float dv;
const float PI = 3.14159265359;
const float SQRT3 = 1.73205080757;

vec3 bCurve(float t) {
    return (1 - t) * (1 - t) * CONTROL_POINTS[0] +
        2 * (1 - t) * t * CONTROL_POINTS[1] +
        t * t * CONTROL_POINTS[2];
}

float dot2(vec2 v) {
    return dot(v, v);
}

// FUNCTION ADAPTED FROM https://www.shadertoy.com/view/MlKcDD
float bCurveClosest(vec2 pos, out float dist) {
    vec2 A = CONTROL_POINTS[0].xz, B = CONTROL_POINTS[1].xz, C = CONTROL_POINTS[2].xz;
    vec2 a = B - A;
    vec2 b = A - 2.0 * B + C;
    vec2 c = a * 2.0;
    vec2 d = A - pos;
    float kk = 1.0 / dot(b, b);
    float kx = kk * dot(a, b);
    float ky = kk * (2.0 * dot(a, a) + dot(d, b)) / 3.0;
    float kz = kk * dot(d, a);
    float p = ky - kx * kx;
    float p3 = p * p * p;
    float q = kx * (2.0 * kx * kx - 3.0 * ky) + kz;
    float h = q * q + 4.0 * p3;
    float t = 0.0;

    if (h >= 0.0) {
        h = sqrt(h);
        vec2 x = (vec2(h, -h) - q) / 2.0;
        vec2 uv = sign(x) * pow(abs(x), vec2(1.0 / 3.0));
        t = clamp(uv.x + uv.y - kx, 0.0, 1.0);
        dist = dot2(d + (c + b * t) * t);
    } else {
        float z = sqrt(-p);
        float v = acos(q / (p * z * 2.0)) / 3.0;
        float m = cos(v);
        float n = sin(v) * SQRT3;
        vec3 ts = clamp(vec3(m + m, -n - m, n - m) * z - kx, 0.0, 1.0);
        float dist1 = dot2(d + (c + b * ts.x) * ts.x);
        float dist2 = dot2(d + (c + b * ts.y) * ts.y);
        // the third root cannot be the closest
        // float dist3 = dot2(d + (c + b * ts.z) * ts.z);
        t = (dist1 < dist2) ? ts.x : ts.y;
        dist = min(dist1, dist2);
    }

    return t;
}

float softplus(float x) {
    return log(1 + exp(x));
}

float addNoise(float h, float i, float j, float d) {
    return h + 1.5 * texture(PERLIN_NOISE, vec2(i * 1.0 / NUM_INSTANCES, j / 128.0)).x * d * d + 2 * max(d - 1, 0);
}

// hight without canal and with noise
float height1(float u, float v, float i, float j) {
    float r = 10.0;
    float d = length(vec2(u, v)) / r;
    float maxh = 10;
    return addNoise((d < 1 ? (d * d / 2) : (2 - (2 - d) * (2 - d)) / 2) * maxh, i, j, d);
}

float bump(float d) {
    return d < 1 ? exp(1.0 - 1.0 / (1 - d * d)) : 0;
}

float canalHeight(float u, float v, float h) {
    float dist;
    float t_closest = bCurveClosest(vec2(u, v), dist);
    // float h_closest = height2(closest.x, closest.y);
    float h_closest = bCurve(t_closest).y;
    // return mix(h_closest, h, 1 - sin(PI / 2.0 - smoothstep(0, 1, dist / CANAL_RADIUS) * PI / 2.0));
    return h - (h - h_closest) * bump(dist / CANAL_RADIUS);
}

// height with canal and noise
float height(float u, float v, int i, int j) {
    float h = height1(u, v, i, j);
    return canalHeight(u, v, h);
    // return 5;
}

void emitVertexAt(int i, int j) {
    float u = i * du - WIDTH / 2.0;
    float v = j * dv - LENGTH / 2.0;

    vec3 uhv = vec3(u, height(u, v, i, j), v);
    vec3 next_u = vec3(u + du, height(u + du, v, i + 1, j), v);
    vec3 next_v = vec3(u, height(u, v + dv, i, j + 1), v + dv);

    vec4 world_pos = gl_in[0].gl_Position + vec4(uhv, 0.0);
    gl_Position = WIST_PROJECTION_MATRIX * WIST_VIEW_MATRIX * world_pos;
    frag_normal = normalize(cross(next_v - uhv, next_u - uhv));
    frag_color = gs_color[0];
    frag_tex_coord = vec2(i * 1.0 / NUM_INSTANCES, j / 128.0);
    frag_world_pos = world_pos.xyz;
    EmitVertex();
}

void main() {
    du = WIDTH / NUM_INSTANCES;
    dv = LENGTH / 127.0;

    int i = gs_instance_id[0];
    for (int j = 0; j < 128; ++j) {
        emitVertexAt(i, j);
        emitVertexAt(i + 1, j);
    }
    EndPrimitive();
}
