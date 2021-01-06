#version 450

#extension GL_EXT_scalar_block_layout : enable

#include "material.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec4 fragClipSpace;
layout(location = 3) out vec2 fragTextureCoords;

const float tiling = 10;

layout(push_constant, scalar) uniform PushConstant
{
    vec3 cameraPos;
    mat4 view;
    mat4 projection;

    vec4 clippingPlane;

    mat4 model;
}
pushConstant;

void main()
{
    vec4 worldPos = pushConstant.model * vec4(pos, 1);
    fragWorldPos = worldPos.xyz;
    fragNorm = normalize((pushConstant.model * vec4(norm, 0)).xyz);
    vec4 clipSpace = pushConstant.projection * pushConstant.view * worldPos;
    fragClipSpace = clipSpace;
    fragTextureCoords = vec2(pos.x / 2 + 0.5, pos.z / 2 + 0.5) * tiling;

    gl_ClipDistance[0] = dot(worldPos, pushConstant.clippingPlane);
    gl_Position = clipSpace;
}