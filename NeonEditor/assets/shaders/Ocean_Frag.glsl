#version 450

#define MAX_LIGHT_COUNT 100u

#define ONE_OVER_4PI 0.0795774715459476

layout (location = 0) out vec4 o_Color;

layout (location = 0) in vec3 v_Normal;
layout (location = 1) in vec3 v_WorldPosition;

struct Light
{
	float Strength;
	vec4 Direction;
	vec4 Radiance;
};

layout (std140, binding = 0) uniform CameraUBO
{
    mat4 u_Model;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
};

layout (std140, binding = 6) uniform LightUBO
{
	uint u_Count;
	Light u_Lights[MAX_LIGHT_COUNT];
};

layout (binding = 7) uniform samplerCube u_EnvRadianceTex;

void main()
{
	vec3 n = normalize(v_Normal);
	vec3 v = normalize(v_WorldPosition - u_CameraPosition.xyz);
	vec3 l = reflect(-v, n);

	float F0 = 0.020018673;
	float F = F0 + (1.0 - F0) * pow(1.0 - dot(n, l), 5.0);

	vec3 refl = texture(u_EnvRadianceTex, l).rgb;

	o_Color = vec4(vec3(dot(n, vec3(1.0))), 1.0) * vec4(0.05, 0.22, 0.4, 1.0);
}
