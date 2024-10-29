#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform localUniform {
    vec4 color;
} object_ubo;

layout(location = 0) in flat int mode;
layout(location = 1) in struct dto {
    vec2 tex;
} in_dto;

void main() {
    outColor = object_ubo.color;
}
