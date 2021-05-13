#version 450

#define MAX_LIGHT_COUNT 100u

#define ONE_OVER_4PI 0.0795774715459476

layout (location = 0) out vec4 o_Color;

layout (location = 0) in vec3 v_WorldPosition;
layout (location = 1) in vec2 v_LocalPosition;

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

layout (binding = 3) uniform sampler2D u_Gradients;

layout (std140, binding = 4) uniform LightUBO
{
	uint u_Count;
	Light u_Lights[MAX_LIGHT_COUNT];
};

layout (binding = 5) uniform samplerCube u_EnvRadianceTex;

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

void main()
{
	vec3 oceanColor = vec3(0.2122, 0.6105, 1.0000);

	vec4 grad = texture(u_Gradients, v_LocalPosition);
	vec3 N = normalize(grad.xyz);
	vec3 V = u_CameraPosition.xyz - v_WorldPosition;
	vec3 L = reflect(-normalize(V), N);

	PBRProperties.Albedo = vec4(0.05, 0.22, 0.4, 1.0);
    PBRProperties.Normal = N;
    PBRProperties.Metalness = 0.1;
    PBRProperties.Roughness = max(0.5, 1.0 - 1.0 / (1.0 + length(V) * 0.001));

	o_Color = vec4(Lighting(normalize(V)), 1.f);
}
