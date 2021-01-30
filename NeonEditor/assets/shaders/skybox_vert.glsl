#version 450

layout(location = 0) in vec2 a_Position;

layout (location = 0) out vec3 v_Position;

layout (std140, binding = 0) uniform CameraUBO
{
    mat4 u_InverseVP;
};

void main()
{
	gl_Position = vec4(a_Position, 1.0, 1.0);
	v_Position = (u_InverseVP * vec4(a_Position, 1.0, 1.0)).xyz;
}
