#version 450

layout(location = 0) in vec3 a_Pos;

layout(location = 0) out vec3 v_WorldPos;

layout (std140, binding = 0) uniform Camera
{
    mat4 u_Model;
    mat4 u_ViewProjection;
};

layout(push_constant) uniform PushConstant
{
    vec4 u_ClippingPlane;
};

void main()
{
    vec4 worldPos = u_Model * vec4(a_Pos, 1.0);
    v_WorldPos = (worldPos).xyz;

    gl_ClipDistance[0] = dot(worldPos, u_ClippingPlane);

    gl_Position = u_ViewProjection * worldPos;
}