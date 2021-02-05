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

layout (binding = 4) uniform sampler2D u_AlbedoTextures[];
layout (binding = 5) uniform sampler2D u_NormalTextures[];
layout (binding = 6) uniform sampler2D u_RoughnessTextures[];
layout (binding = 7) uniform sampler2D u_MetalnessTextures[];

const float PI = 3.14159265359;
const float GAMMA = 2.2;
const float EPSILON = 0.00001;

const vec3 FDielectric = vec3(0.04);

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

void main()
{
    vec4 albedo =  u_Materials[v_MaterialIndex].HasAlbedoTexture < 0.5 ? u_Materials[v_MaterialIndex].AlbedoColor : texture(u_AlbedoTextures[v_MaterialIndex], v_TexCoord);
    vec3 normal = v_Normal;
    float metalness = u_Materials[v_MaterialIndex].HasMetalnessTex < 0.5 ? u_Materials[v_MaterialIndex].Metalness : texture(u_MetalnessTextures[v_MaterialIndex], v_TexCoord).r;
    float roughness = u_Materials[v_MaterialIndex].HasRoughnessTex < 0.5 ? u_Materials[v_MaterialIndex].Roughness : texture(u_RoughnessTextures[v_MaterialIndex], v_TexCoord).r;

    if (u_Materials[v_MaterialIndex].HasNormalTex >= 0.5)
    {
        normal = 2.0 * texture(u_NormalTextures[v_MaterialIndex], v_TexCoord).rgb - 1.0;
        normal = v_WorldNormals * normal;
    }

    normal = normalize(normal);

    vec3 totalColor = vec3(0, 0, 0);
    uint lightCount = clamp(u_Count, 0, MAX_LIGHT_COUNT - 1);

    vec3 view = normalize(u_CameraPosition.xyz - v_WorldPosition);
	for (int i = 0; i < lightCount; i++)
	{
        vec3 light = -normalize(u_Lights[i].Direction.xyz);
        vec3 halfway = normalize(view + light);
                
        // Cook-Torrance BRDF
        vec3 F0 = mix(FDielectric, albedo.rgb, metalness);
        float NDF = NDFTrowbridgeReitzGGX(normal, halfway, roughness);
        float geometry = GeometrySmith(normal, view, light, roughness);
        vec3 fresnel = FresnelSchlick(max(dot(halfway, view), 0.0), F0);
        vec3 kD = (vec3(1.0) - fresnel) * (1.0 - metalness);
                
        vec3 numerator = NDF * geometry * fresnel;
        float NdotL = max(dot(normal, light), 0.0);
        float denominator = 4.0 * max(dot(normal, view), 0.0) * NdotL;
        vec3  specular = numerator / max(denominator, EPSILON);
                    
        vec3 color = u_Lights[i].Radiance.rgb * (kD * albedo.rgb / PI + specular) * NdotL;
                
        totalColor += u_Lights[i].Strength * color;
	}

	o_Color = vec4(pow(totalColor, vec3(1.0 / GAMMA)), albedo.a);
}
