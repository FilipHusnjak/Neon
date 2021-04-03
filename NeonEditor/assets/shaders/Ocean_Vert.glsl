#version 450

layout (location = 0) in vec2 a_Position;

layout (location = 0) out vec3 v_Normal;
layout (location = 1) out vec3 v_WorldPosition;

layout (std140, binding = 0) uniform CameraUBO
{
    mat4 u_Model;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
};

layout (binding = 1) uniform sampler2D u_DisplacementY;
layout (binding = 2) uniform sampler2D u_DisplacementX;
layout (binding = 3) uniform sampler2D u_DisplacementZ;
layout (binding = 4) uniform sampler2D u_DerivativesX;
layout (binding = 5) uniform sampler2D u_DerivativesZ;

void main()
{
    float dy = texture(u_DisplacementY, a_Position).r;
    float dx = texture(u_DisplacementX, a_Position).r;
    float dz = texture(u_DisplacementZ, a_Position).r;
    float dydx = texture(u_DerivativesX, a_Position).r;
    float dydz = texture(u_DerivativesZ, a_Position).r;

    v_Normal = vec3(-dydx, 1.0, -dydz);
    
    vec4 pos = vec4(10.0 * a_Position.x + 10.0 * dx, 10.0 * dy, 10.0 * a_Position.y + 10.0 * dz, 1.0);
    vec4 worldPosition = u_Model * pos;
    v_WorldPosition = worldPosition.xyz / worldPosition.w;
    gl_Position = u_ViewProjection * worldPosition;
}
