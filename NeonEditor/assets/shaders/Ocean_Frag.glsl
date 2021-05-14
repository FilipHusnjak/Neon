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

layout (std140, binding = 7) uniform LightUBO
{
	uint u_Count;
	Light u_Lights[MAX_LIGHT_COUNT];
};

layout (binding = 8) uniform samplerCube u_EnvRadianceTex;

void main()
{
    vec3 oceanColor = vec3(0.0000, 0.0103, 0.0331);
    const vec3 sunColor			= vec3(1.0, 1.0, 0.47);
	const vec3 sundir			= vec3(0.603, 0.240, -0.761);

	vec3 N = normalize(v_Normal);
	vec3 V = normalize(u_CameraPosition.xyz - v_WorldPosition);
	vec3 L = reflect(-V, N);

    float F0 = 0.020018673;
	float F = F0 + (1.0 - F0) * pow(1.0 - dot(N, L), 5.0);

	vec3 refl = texture(u_EnvRadianceTex, L).rgb;

	// Reinhard tonemapping operator.
    const float pureWhite = 1.0;
	float luminance = dot(refl, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale color by ratio of average luminances.
	//refl = (mappedLuminance / luminance) * refl;

	// tweaked from ARM/Mali's sample
	float color_mod = 1.0 + 3.0 * smoothstep(1.2, 1.8, 0.0);

    const float rho = 0.3;
	const float ax = 0.2;
	const float ay = 0.1;

	vec3 h = sundir + V;
	vec3 x = cross(sundir, N);
	vec3 y = cross(x, N);

	float mult = (ONE_OVER_4PI * rho / (ax * ay * sqrt(max(1e-5, dot(sundir, N) * dot(V, N)))));
	float hdotx = dot(h, x) / ax;
	float hdoty = dot(h, y) / ay;
	float hdotn = dot(h, N);

	float spec = mult * exp(-((hdotx * hdotx) + (hdoty * hdoty)) / (hdotn * hdotn));

	o_Color = vec4(mix(oceanColor, refl * color_mod, F) + sunColor * spec, 1.0);
}
