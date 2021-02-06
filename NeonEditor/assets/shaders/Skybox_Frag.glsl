#version 450

layout (location = 0) out vec4 o_Color;

layout (binding = 1) uniform samplerCube u_Cubemap;

layout (location = 0) in vec3 v_Position;

const float GAMMA = 2.2;

void main()
{
	o_Color =  vec4(pow(texture(u_Cubemap, v_Position).xyz, vec3(1.0 / GAMMA)), 1.0);
}
