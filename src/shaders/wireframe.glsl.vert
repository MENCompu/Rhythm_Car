#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex;

layout(push_constant, std430) uniform Consts {
    mat4 PVM;
} c;

layout(location = 0) out int mode;
layout(location = 1) out dto {
    vec2 tex;
} out_dto;

void main() {
    out_dto.tex = tex;
    gl_Position = c.PVM * vec4(pos, 1.0f);
}
