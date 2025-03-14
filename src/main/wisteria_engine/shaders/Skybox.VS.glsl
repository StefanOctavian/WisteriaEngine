#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

uniform mat4 WIST_MODEL_MATRIX;
uniform mat4 WIST_VIEW_MATRIX;
uniform mat4 WIST_PROJECTION_MATRIX;

out vec3 frag_text_coord;

void main()
{
    mat4 MVP = WIST_PROJECTION_MATRIX * WIST_VIEW_MATRIX * WIST_MODEL_MATRIX;
    frag_text_coord = normalize(v_position);
    gl_Position = MVP * vec4(v_position, 1); 
}
