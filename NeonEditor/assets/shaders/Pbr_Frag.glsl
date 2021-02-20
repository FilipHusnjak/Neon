#version 450

#extension GL_EXT_nonuniform_qualifier : enable

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

const float PI = 3.141592;
const float Gamma = 2.2;
const float Epsilon = 0.00001;

const vec3 FDielectric = vec3(0.04);

struct
{
    vec4 Albedo;
    vec3 Normal;
    float Metalness;
    float Roughness;
} PBRProperties;

// Normal distribution function
float NDFTrowbridgeReitzGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

    float NdotH = max(dot(N, H), 0.0);
    float denom = NdotH * NdotH * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Geometry function
float GeometrySchlickGGX(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}
// Take both view and light direction into account using Smith's method
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return GeometrySchlickGGX(max(dot(N, L), 0.0), k) *
           GeometrySchlickGGX(max(dot(N, V), 0.0), k);
}

// Shlick's approximation of the Fresnel factor.
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow (1.0 - cosTheta, 5.0);
}
vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 Lighting(vec3 V)
{
    uint lightCount = clamp(u_Count, 0, MAX_LIGHT_COUNT - 1);
    vec3 result = vec3(0.0);
    for (int i = 0; i < lightCount; i++)
	{
        vec3 L = normalize(u_Lights[i].Direction.xyz);      
        vec3 H = normalize(V + L);

        // Cook-Torrance BRDF
        vec3 F0 = mix(FDielectric, PBRProperties.Albedo.rgb, PBRProperties.Metalness);
        float NDF = NDFTrowbridgeReitzGGX(PBRProperties.Normal, H, PBRProperties.Roughness);
        float geometry = GeometrySmith(PBRProperties.Normal, V, L, PBRProperties.Roughness);
        vec3 fresnel = FresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 kD = (vec3(1.0) - fresnel) * (1.0 - PBRProperties.Metalness);
        vec3 diffuse = kD * PBRProperties.Albedo.rgb / PI;
                
        vec3 numerator = NDF * geometry * fresnel;
        float NdotL = max(dot(PBRProperties.Normal, L), 0.0);
        float denominator = 4.0 * max(dot(PBRProperties.Normal, V), 0.0) * NdotL;
        vec3 specular = numerator / max(denominator, Epsilon);

        result += u_Lights[i].Strength * u_Lights[i].Radiance.rgb * (diffuse + specular) * NdotL;
	}

    return result;
}

vec3 IBL(vec3 V)
{
    float NdotV = dot(PBRProperties.Normal, V);
    float NdotVPositive = max(NdotV, 0.0);

	vec3 irradiance = texture(u_EnvIrradianceTex, PBRProperties.Normal).rgb;
    vec3 F0 = mix(FDielectric, PBRProperties.Albedo.rgb, PBRProperties.Metalness);
	vec3 F = FresnelSchlickRoughness(F0, NdotVPositive, PBRProperties.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - PBRProperties.Metalness);
	vec3 diffuseIBL = PBRProperties.Albedo.rgb * irradiance;

	int envRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);
	vec3 R = 2.0 * NdotV * PBRProperties.Normal - V;
	vec3 specularIrradiance = textureLod(u_EnvRadianceTex, R, PBRProperties.Roughness * envRadianceTexLevels).rgb;

	vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(NdotVPositive, PBRProperties.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

void main()
{
    PBRProperties.Albedo =  u_Materials[v_MaterialIndex].UseAlbedoMap < 0.5 ? u_Materials[v_MaterialIndex].AlbedoColor : texture(u_AlbedoTextures[v_MaterialIndex], v_TexCoord);
    PBRProperties.Normal = v_Normal;
    PBRProperties.Metalness = u_Materials[v_MaterialIndex].UseMetalnessMap < 0.5 ? u_Materials[v_MaterialIndex].Metalness : texture(u_MetalnessTextures[v_MaterialIndex], v_TexCoord).r;
    PBRProperties.Roughness = u_Materials[v_MaterialIndex].UseRoughnessMap < 0.5 ? u_Materials[v_MaterialIndex].Roughness : texture(u_RoughnessTextures[v_MaterialIndex], v_TexCoord).r;
    PBRProperties.Roughness = max(PBRProperties.Roughness, 0.01); // Minimum roughness of 0.01 to keep specular highlight

    if (u_Materials[v_MaterialIndex].UseNormalMap >= 0.5)
    {
        PBRProperties.Normal = 2.0 * texture(u_NormalTextures[v_MaterialIndex], v_TexCoord).rgb - 1.0;
        PBRProperties.Normal = v_WorldNormals * PBRProperties.Normal;
    }

    PBRProperties.Normal = normalize(PBRProperties.Normal);

    vec3 totalColor = vec3(0.0);
    vec3 V = normalize(u_CameraPosition.xyz - v_WorldPosition);
	totalColor += Lighting(V);
    totalColor += IBL(V);

	o_Color = vec4(totalColor, PBRProperties.Albedo.a);
}
