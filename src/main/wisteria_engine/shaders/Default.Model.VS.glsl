#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
layout(location = 3) in vec3 v_color;

uniform mat4 WIST_MODEL_MATRIX;

out vec3 frag_normal;
out vec3 frag_color;
out vec2 frag_tex_coord;
out int frag_instance_id;

out vec3 gs_normal;
out vec3 gs_color;
out vec2 gs_tex_coord;
out int gs_instance_id;

void main()
{
    frag_normal = mat3(WIST_MODEL_MATRIX) * v_normal;
    gs_normal = frag_normal;

    frag_color = v_color;
    gs_color = frag_color;

    frag_tex_coord = v_texture_coord;
    gs_tex_coord = frag_tex_coord;

    frag_instance_id = gl_InstanceID;
    gs_instance_id = frag_instance_id;

    gl_Position = WIST_MODEL_MATRIX * vec4(v_position, 1.0);
}
