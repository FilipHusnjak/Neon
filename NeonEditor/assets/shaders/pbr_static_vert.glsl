#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Binormal;
layout (location = 4) in uint a_MaterialIndex;
layout (location = 5) in vec2 a_TexCoord;

layout (location = 0) flat out uint v_MaterialIndex;
layout (location = 1) out vec2 v_TexCoord;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_Model;
    mat4 u_ViewProjection;
};

void main()
{
    v_MaterialIndex = a_MaterialIndex;
    v_TexCoord = a_TexCoord;

    gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}
