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

void main()
{
	vec3 oceanColor = vec3(0.2122, 0.6105, 1.0000);

	vec4 grad = texture(u_Gradients, v_LocalPosition);
	vec3 N = normalize(grad.xyz);
	vec3 V = u_CameraPosition.xyz - v_WorldPosition;
	vec3 L = reflect(-normalize(V), N);

	// NOTE: also defined in vertex shader
	const vec3 sunColor			= vec3(1.0, 1.0, 0.47);
	const vec3 perlinFrequency	= vec3(1.12, 0.59, 0.23);
	const vec3 perlinGradient	= vec3(0.014, 0.016, 0.022);
	const vec3 sundir			= vec3(0.603, -0.761, 0.240);

	float F0 = 0.020018673;
	float F = F0 + (1.0 - F0) * pow(1.0 - dot(N, L), 5.0);

	vec3 refl = texture(u_EnvRadianceTex, L).rgb;

	// Reinhard tonemapping operator.
	const float pureWhite = 1.0;
	float luminance = dot(refl, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale color by ratio of average luminances.
	vec3 mappedColor = (mappedLuminance / luminance) * refl;

	// tweaked from ARM/Mali's sample
	float turbulence = max(1.6 - grad.w, 0.0);
	float color_mod = 1.0 + 3.0 * smoothstep(1.2, 1.8, turbulence);

	// some additional specular (Ward model)
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

	o_Color = vec4(oceanColor + sunColor * spec, 1.0);
}
