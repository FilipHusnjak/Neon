#version 450

layout (location = 0) in vec2 a_Position;

layout (location = 0) out vec3 v_WorldPosition;
layout (location = 1) out vec2 v_LocalPosition;

layout (std140, binding = 0) uniform CameraUBO
{
    mat4 u_Model;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
};

layout (std140, binding = 1) uniform PropertiesUBO
{
    float u_L;
};

layout (binding = 2) uniform sampler2D u_Displacement;

void main()
{
    vec3 disp = texture(u_Displacement, a_Position).rgb;
    float dy = disp.r;
    float dx = disp.g;
    float dz = disp.b;
    
    vec4 pos = vec4(u_L * a_Position.x + 1.3 * dx, dy, u_L * a_Position.y + 1.3 * dz, 1.0);
    vec4 worldPosition = u_Model * pos;
    v_WorldPosition = worldPosition.xyz / worldPosition.w;
    v_LocalPosition = a_Position;
    gl_Position = u_ViewProjection * worldPosition;
}
