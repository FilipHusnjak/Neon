#version 450

#define MAX_LIGHT_COUNT 100u

layout (location = 0) out vec4 o_Color;

struct Light
{
	float Strength;
	vec4 Direction;
	vec4 Radiance;
};

layout (std140, binding = 4) uniform LightUBO
{
	uint u_Count;
	Light u_Lights[MAX_LIGHT_COUNT];
};

void main()
{
	o_Color =  vec4(1.0, 0.0, 0.0, 1.0);
}
