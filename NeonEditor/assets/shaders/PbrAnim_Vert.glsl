#version 450

#define MAX_BONES_PER_VERTEX 10

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Binormal;
layout (location = 4) in uint a_MaterialIndex;
layout (location = 5) in vec2 a_TexCoord;
layout (location = 6) in uint a_BoneIds[MAX_BONES_PER_VERTEX];
layout (location = 6 + MAX_BONES_PER_VERTEX) in float a_BoneWeights[MAX_BONES_PER_VERTEX];

layout (location = 0) out vec3 v_WorldPosition;
layout (location = 1) out vec3 v_Normal;
layout (location = 2) out vec2 v_TexCoord;
layout (location = 3) flat out uint v_MaterialIndex;
layout (location = 4) out mat3 v_WorldNormals;

layout (std140, binding = 0) uniform CameraUBO
{
    mat4 u_Model;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
};

layout (std140, binding = 1) readonly buffer BonesUBO
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
	
	mat4 worldTransform = u_Model * boneTransform;

    vec4 worldPosition = worldTransform * vec4(a_Position, 1.0);
    v_WorldPosition = worldPosition.xyz;
	v_Normal = normalize((worldTransform * vec4(a_Normal, 0.0)).xyz);
    v_TexCoord = a_TexCoord;
    v_MaterialIndex = a_MaterialIndex;
    v_WorldNormals = mat3(u_Model) * mat3(a_Tangent, a_Binormal, a_Normal);

    gl_Position = u_ViewProjection * worldPosition;
}