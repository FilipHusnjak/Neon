#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Binormal;
layout (location = 4) in uint a_MaterialIndex;
layout (location = 5) in vec2 a_TexCoord;

layout (location = 0) flat out uint fragMaterialIndex;
layout (location = 1) out vec2 fragTexCoord;

layout (std140, binding = 0) uniform Camera
{
    mat4 model;
    mat4 view;
    mat4 projection;
};

void main()
{
    fragMaterialIndex = a_MaterialIndex;
    fragTexCoord = a_TexCoord;
    gl_Position = projection * view * model * vec4(a_Position, 1.0);
}
