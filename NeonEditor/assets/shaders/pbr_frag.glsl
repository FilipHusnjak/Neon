#version 450

#extension GL_EXT_nonuniform_qualifier : enable

struct DirectionalLight
{
	float Strength;
	vec4 Direction;
	vec4 Radiance;
};

#define MAX_DIRECTIONAL_LIGHT_COUNT 100u

layout (location = 0) flat in uint v_MaterialIndex;
layout (location = 1) in vec2 v_TexCoord;
layout (location = 2) in vec3 v_Normal;

layout (location = 0) out vec4 o_Color;

layout (std140, binding = 2) uniform LightUBO
{
	uint u_Count;
	DirectionalLight u_DirectionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
};

layout (std140, binding = 3) uniform MaterialUBO
{
    vec4 AlbedoColor;
	float HasAlbedoTexture;
	float HasNormalTex;
	float Metalness;
	float HasMetalnessTex;
	float Roughness;
	float HasRoughnessTex;
} u_Materials[];

layout (binding = 4) uniform sampler2D u_AlbedoTextures[];
layout (binding = 5) uniform sampler2D u_NormalTextures[];
layout (binding = 6) uniform sampler2D u_RoughnessTextures[];
layout (binding = 7) uniform sampler2D u_MetalnessTextures[];

void main()
{
	vec4 diffuseColor =  u_Materials[v_MaterialIndex].HasAlbedoTexture < 0.5f ? vec4(u_Materials[v_MaterialIndex].AlbedoColor.xyz, 1.f) : texture(u_AlbedoTextures[v_MaterialIndex], v_TexCoord);
	vec3 outColor = vec3(0, 0, 0);
	uint lightCount = clamp(u_Count, 0, MAX_DIRECTIONAL_LIGHT_COUNT - 1);
	for (int i = 0; i < lightCount; i++)
	{
		outColor += u_DirectionalLights[i].Strength * max(dot(v_Normal, u_DirectionalLights[i].Direction.xyz), 0.0) * u_DirectionalLights[i].Radiance.xyz * diffuseColor.xyz;
	}
	outColor = clamp(outColor, 0, 1);
	o_Color = vec4(outColor, diffuseColor.w);
}
