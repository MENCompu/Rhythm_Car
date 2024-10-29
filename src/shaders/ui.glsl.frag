#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform localUniform {
    vec4 diffuse;
} object_ubo;

layout(set = 0, binding = 1) uniform sampler2D diffuseSampler;
layout(set = 0, binding = 2) uniform sampler2D normalSampler;

layout(location = 0) in flat int mode;
layout(location = 1) in struct dto {
    vec2 tex;
} in_dto;

void main() {
    //outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    outColor = object_ubo.diffuse * texture(diffuseSampler, in_dto.tex);
}
