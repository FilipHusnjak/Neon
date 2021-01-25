#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) flat in uint v_MaterialIndex;
layout (location = 1) in vec2 v_TexCoord;

layout (location = 0) out vec4 o_Color;

layout (binding = 2) uniform Material
{
    vec4 AlbedoColor;
	float HasAlbedoTexture;
	float HasNormalTex;
	float Metalness;
	float HasMetalnessTex;
	float Roughness;
	float HasRoughnessTex;
} u_Materials[];

layout (binding = 3) uniform sampler2D u_AlbedoTextures[];
layout (binding = 4) uniform sampler2D u_NormalTextures[];
layout (binding = 5) uniform sampler2D u_RoughnessTextures[];
layout (binding = 6) uniform sampler2D u_MetalnessTextures[];

void main()
{
	o_Color = u_Materials[v_MaterialIndex].HasAlbedoTexture < 0.5f ? vec4(u_Materials[v_MaterialIndex].AlbedoColor.xyz, 1.f) : texture(u_AlbedoTextures[v_MaterialIndex], v_TexCoord);
}
