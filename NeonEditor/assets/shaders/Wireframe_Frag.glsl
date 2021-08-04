#version 450

struct Light
{
	float Strength;
	vec4 Direction;
	vec4 Radiance;
};

#define MAX_LIGHT_COUNT 100u

layout (location = 0) in vec3 v_WorldPosition;
layout (location = 1) in vec3 v_Normal;
layout (location = 2) in vec2 v_TexCoord;
layout (location = 3) flat in uint v_MaterialIndex;
layout (location = 4) in mat3 v_WorldNormals;

layout (location = 0) out vec4 o_Color;

layout (std140, binding = 0) uniform CameraUBO
{
    mat4 u_Model;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
};

layout (std140, binding = 2) uniform LightUBO
{
	uint u_Count;
	Light u_Lights[MAX_LIGHT_COUNT];
};

layout (std140, binding = 3) uniform MaterialUBO
{
    vec4 AlbedoColor;
	float UseAlbedoMap;
	float UseNormalMap;
	float Metalness;
	float UseMetalnessMap;
	float Roughness;
	float UseRoughnessMap;
} u_Materials[];

// Material textures
layout (binding = 4) uniform sampler2D u_AlbedoTextures[];
layout (binding = 5) uniform sampler2D u_NormalTextures[];
layout (binding = 6) uniform sampler2D u_RoughnessTextures[];
layout (binding = 7) uniform sampler2D u_MetalnessTextures[];

// Environment maps
layout (binding = 8) uniform samplerCube u_EnvRadianceTex;
layout (binding = 9) uniform samplerCube u_EnvIrradianceTex;

// BRDF LUT
layout (binding = 10) uniform sampler2D u_BRDFLUTTexture;

void main()
{
	o_Color = vec4(1.0, 0.0, 0.0, 1.0);
}
