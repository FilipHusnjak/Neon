#version 450

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;

layout(location = 0) out vec3 v_WorldPos;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec4 v_ClipSpace;
layout(location = 3) out vec2 v_TexCoords;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_Model;
    mat4 u_ViewProjection;
};

layout(push_constant) uniform PushConstant
{
    vec4 u_CameraPosition;

    vec4 u_LightDirection;
    vec4 u_LightPosition;

    vec4 u_ClippingPlane;

    int u_IsPointLight;
    float u_LightIntensity;

    float u_MoveFactor;

    float u_Near;
    float u_Far;
};

const float tiling = 10;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Pos, 1);
    v_WorldPos = worldPos.xyz;
    v_Normal = normalize((u_Model * vec4(a_Normal, 0)).xyz);
    vec4 clipSpace = u_ViewProjection * worldPos;
    v_ClipSpace = clipSpace;
    v_TexCoords = vec2(a_Pos.x / 2 + 0.5, a_Pos.z / 2 + 0.5) * tiling;

    gl_ClipDistance[0] = dot(worldPos, u_ClippingPlane);
    gl_Position = clipSpace;
}