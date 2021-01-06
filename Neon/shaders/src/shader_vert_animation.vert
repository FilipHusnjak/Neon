#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable

#include "material.glsl"

#define MAX_BONES_PER_VERTEX 10

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in int matID;
layout(location = 5) in uint boneIDs[MAX_BONES_PER_VERTEX];
layout(location = 5 + MAX_BONES_PER_VERTEX) in float boneWeights[MAX_BONES_PER_VERTEX];

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec3 fragWorldPos;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) flat out int fragMatID;
layout(location = 5) out vec4 clipSpace;

layout(set = 0, binding = 2, scalar) readonly buffer BoneBuffer
{
    mat4 boneTransforms[];
};

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
    mat4 boneTransform = mat4(0.0);
    for (int i = 0; i < MAX_BONES_PER_VERTEX; i++)
    {
        boneTransform += boneTransforms[boneIDs[i]] * boneWeights[i];
    }
    fragColor = color;
    mat4 worldTransform = pushConstant.model * boneTransform;
    fragNorm = normalize((worldTransform * vec4(norm, 0)).xyz);
    vec4 worldPos = worldTransform * vec4(pos, 1);
    fragWorldPos = worldPos.xyz;
    fragTexCoord = texCoord;
    fragMatID = matID;

    clipSpace = pushConstant.projection * pushConstant.view * worldPos;

    gl_ClipDistance[0] = dot(worldPos, pushConstant.clippingPlane);

    gl_Position = pushConstant.projection * pushConstant.view * worldPos;
}