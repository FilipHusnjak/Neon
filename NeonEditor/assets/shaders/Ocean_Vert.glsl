#version 450

layout (location = 0) in vec2 a_Position;

layout (location = 0) out vec3 v_Normal;
layout (location = 1) out vec3 v_WorldPosition;
layout (location = 2) out float v_J;
layout (location = 3) out vec3 v_Foam;

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

layout (binding = 2) uniform sampler2D u_DisplacementY;
layout (binding = 3) uniform sampler2D u_DisplacementX;
layout (binding = 4) uniform sampler2D u_DisplacementZ;
layout (binding = 5) uniform sampler2D u_DerivativesYX;
layout (binding = 6) uniform sampler2D u_DerivativesYZ;
layout (binding = 7) uniform sampler2D u_Jacobian;
layout (binding = 8) uniform sampler2D u_Foam;

void main()
{
    float dy = texture(u_DisplacementY, a_Position).r;
    float dx = texture(u_DisplacementX, a_Position).r;
    float dz = texture(u_DisplacementZ, a_Position).r;
    float dydx = texture(u_DerivativesYX, a_Position).r;
    float dydz = texture(u_DerivativesYZ, a_Position).r;

    v_Normal = vec3(-dydx, 1.0, -dydz);

    v_J = texture(u_Jacobian, a_Position).r;

    v_Foam = texture(u_Foam, a_Position).rgb;
    
    vec4 pos = vec4(u_L * a_Position.x + 1.3 * dx, dy, u_L * a_Position.y + 1.3 * dz, 1.0);
    vec4 worldPosition = u_Model * pos;
    v_WorldPosition = worldPosition.xyz / worldPosition.w;
    gl_Position = u_ViewProjection * worldPosition;
}
