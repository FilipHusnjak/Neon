#version 450

layout(location = 0) out vec4 o_Color;

layout (binding = 1) uniform samplerCube u_Cubemap;

layout (location = 0) in vec3 v_Position;

void main()
{
	o_Color =  vec4(texture(u_Cubemap, v_Position).xyz, 1.0);
}
