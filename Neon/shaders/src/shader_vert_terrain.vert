#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable

#include "material.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 mapTexCoord;
layout(location = 4) in vec2 tileTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec3 fragWorldPos;
layout(location = 3) out vec2 fragMapTexCoord;
layout(location = 4) out vec2 fragTileTexCoord;

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
    fragColor = color;
    fragNorm = normalize((pushConstant.model * vec4(norm, 0)).xyz);
    vec4 worldPos = pushConstant.model * vec4(pos, 1);
    fragWorldPos = worldPos.xyz;
    fragMapTexCoord = mapTexCoord;
    fragTileTexCoord = tileTexCoord;

    gl_ClipDistance[0] = dot(worldPos, pushConstant.clippingPlane);

    gl_Position = pushConstant.projection * pushConstant.view * worldPos;
}