#version 450

layout (location = 0) out vec4 o_Color;

layout (binding = 1) uniform samplerCube u_Cubemap;

layout (location = 0) in vec3 v_Position;

const float GAMMA = 2.2;

void main()
{
    vec3 envColor = texture(u_Cubemap, v_Position).rgb;
    const float pureWhite = 1.0;
    // Reinhard tonemapping operator.
	// see: "Photographic Tone Reproduction for Digital Images", eq. 4
	float luminance = dot(envColor, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale color by ratio of average luminances.
	vec3 mappedColor = (mappedLuminance / luminance) * envColor;

	o_Color =  vec4(pow(mappedColor, vec3(1.0 / GAMMA)), 1.0);
}
