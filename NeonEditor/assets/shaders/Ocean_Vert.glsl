#version 450

layout (location = 0) in vec2 a_Position;

layout (std140, binding = 0) uniform CameraUBO
{
    mat4 u_Model;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
};

layout (binding = 1) uniform sampler2D u_DisplacementY;
layout (binding = 2) uniform sampler2D u_DisplacementX;
layout (binding = 3) uniform sampler2D u_DisplacementZ;

void main()
{
    float dy = texture(u_DisplacementY, a_Position).r;
    float dx = texture(u_DisplacementX, a_Position).r;
    float dz = texture(u_DisplacementZ, a_Position).r;

    vec4 pos = vec4(1000.0 * a_Position.x + 12.0 * dx, 10.0 * dy, 1000.0 * a_Position.y + 12.0 * dz, 1.0);
    gl_Position = u_ViewProjection * u_Model * pos;
}
