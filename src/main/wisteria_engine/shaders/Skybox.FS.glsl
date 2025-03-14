#version 330

in vec3 frag_text_coord;

uniform samplerCube TEXTURE_CUBEMAP;

layout(location = 0) out vec4 out_color;

void main() {
    vec3 color = texture(TEXTURE_CUBEMAP, frag_text_coord).xyz;
    out_color = vec4(color, 0);
}