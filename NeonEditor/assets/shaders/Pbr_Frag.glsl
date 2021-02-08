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
	float HasAlbedoTexture;
	float HasNormalTex;
	float Metalness;
	float HasMetalnessTex;
	float Roughness;
	float HasRoughnessTex;
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
const float GAMMA = 2.2;
const float EPSILON = 0.00001;

const vec3 FDielectric = vec3(0.04);

struct
{
    vec4 Albedo;
    vec3 Normal;
    float Metalness;
    float Roughness;
} PBRProperties;

// Normal distribution function
float NDFTrowbridgeReitzGGX(vec3 normal, vec3 halfway, float roughness)
{
    float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

    float NdotH = max(dot(normal, halfway), 0.0);
    float denom = NdotH * NdotH * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Geometry function
float GeometrySchlickGGX(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}
// Take both view and light direction into account using Smith's method
float GeometrySmith(vec3 normal, vec3 view, vec3 light, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return GeometrySchlickGGX(max(dot(normal, light), 0.0), k) *
           GeometrySchlickGGX(max(dot(normal, view), 0.0), k);
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

vec3 Lighting(vec3 light, vec3 view)
{
    vec3 halfway = normalize(view + light);

    // Cook-Torrance BRDF
    vec3 F0 = mix(FDielectric, PBRProperties.Albedo.rgb, PBRProperties.Metalness);
    float NDF = NDFTrowbridgeReitzGGX(PBRProperties.Normal, halfway, PBRProperties.Roughness);
    float geometry = GeometrySmith(PBRProperties.Normal, view, light, PBRProperties.Roughness);
    vec3 fresnel = FresnelSchlick(max(dot(halfway, view), 0.0), F0);
    vec3 kD = (vec3(1.0) - fresnel) * (1.0 - PBRProperties.Metalness);
                
    vec3 numerator = NDF * geometry * fresnel;
    float NdotL = max(dot(PBRProperties.Normal, light), 0.0);
    float denominator = 4.0 * max(dot(PBRProperties.Normal, view), 0.0) * NdotL;
    vec3 specular = numerator / max(denominator, EPSILON);

    return (kD * PBRProperties.Albedo.rgb / PI + specular) * NdotL;
}

vec3 IBL(vec3 view)
{
    float NdotV = max(dot(PBRProperties.Normal, view), 0.0);
    vec3 reflection = 2.0 * NdotV * PBRProperties.Normal - view;

	vec3 irradiance = texture(u_EnvIrradianceTex, PBRProperties.Normal).rgb;
    vec3 F0 = mix(FDielectric, PBRProperties.Albedo.rgb, PBRProperties.Metalness);
	vec3 F = FresnelSchlickRoughness(F0, NdotV, PBRProperties.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - PBRProperties.Metalness);
	vec3 diffuseIBL = PBRProperties.Albedo.rgb * irradiance;

	int envRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);
	float NoV = clamp(NdotV, 0.0, 1.0);
	vec3 R = 2.0 * dot(view, PBRProperties.Normal) * PBRProperties.Normal - view;
	vec3 specularIrradiance = textureLod(u_EnvRadianceTex, reflection, (PBRProperties.Roughness) * envRadianceTexLevels).rgb;

	// Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
	vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(NdotV, 1.0 - PBRProperties.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

void main()
{
    PBRProperties.Albedo =  u_Materials[v_MaterialIndex].HasAlbedoTexture < 0.5 ? u_Materials[v_MaterialIndex].AlbedoColor : texture(u_AlbedoTextures[v_MaterialIndex], v_TexCoord);
    PBRProperties.Normal = v_Normal;
    PBRProperties.Metalness = u_Materials[v_MaterialIndex].HasMetalnessTex < 0.5 ? u_Materials[v_MaterialIndex].Metalness : texture(u_MetalnessTextures[v_MaterialIndex], v_TexCoord).r;
    PBRProperties.Roughness = u_Materials[v_MaterialIndex].HasRoughnessTex < 0.5 ? u_Materials[v_MaterialIndex].Roughness : texture(u_RoughnessTextures[v_MaterialIndex], v_TexCoord).r;

    if (u_Materials[v_MaterialIndex].HasNormalTex >= 0.5)
    {
        PBRProperties.Normal = 2.0 * texture(u_NormalTextures[v_MaterialIndex], v_TexCoord).rgb - 1.0;
        PBRProperties.Normal = v_WorldNormals * PBRProperties.Normal;
    }

    PBRProperties.Normal = normalize(PBRProperties.Normal);

    vec3 totalColor = vec3(0, 0, 0);
    uint lightCount = clamp(u_Count, 0, MAX_LIGHT_COUNT - 1);

    vec3 view = normalize(u_CameraPosition.xyz - v_WorldPosition);
	for (int i = 0; i < lightCount; i++)
	{
        vec3 light = -normalize(u_Lights[i].Direction.xyz);      
        vec3 color = u_Lights[i].Radiance.rgb * Lighting(light, view);
        totalColor += u_Lights[i].Strength * color;
	}
    totalColor += IBL(view);

    const float pureWhite = 1.0;
    // Reinhard tonemapping operator.
	// see: "Photographic Tone Reproduction for Digital Images", eq. 4
	float luminance = dot(totalColor, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale color by ratio of average luminances.
	vec3 mappedColor = (mappedLuminance / luminance) * totalColor;

	o_Color = vec4(pow(mappedColor, vec3(1.0 / GAMMA)), PBRProperties.Albedo.a);
}
