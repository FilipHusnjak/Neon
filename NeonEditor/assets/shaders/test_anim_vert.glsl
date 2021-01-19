#version 450

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_BONES_PER_VERTEX 10

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Binormal;
layout (location = 4) in uint a_MaterialIndex;
layout (location = 5) in vec2 a_TexCoord;
layout(location = 6) in uint a_BoneIds[MAX_BONES_PER_VERTEX];
layout(location = 6 + MAX_BONES_PER_VERTEX) in float a_BoneWeights[MAX_BONES_PER_VERTEX];

layout (location = 0) flat out uint v_MaterialIndex;
layout (location = 1) out vec2 v_TexCoord;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_Model;
    mat4 u_ViewProjection;
};

layout(std140, binding = 2) readonly buffer Bones
{
    mat4 u_BoneTransforms[];
};

void main()
{
    mat4 boneTransform = mat4(0.0);
    for (int i = 0; i < MAX_BONES_PER_VERTEX; i++)
    {
        boneTransform += u_BoneTransforms[a_BoneIds[i]] * a_BoneWeights[i];
    }

    v_MaterialIndex = a_MaterialIndex;
    v_TexCoord = a_TexCoord;

    gl_Position = u_ViewProjection * u_Model * boneTransform * vec4(a_Position, 1.0);
}
