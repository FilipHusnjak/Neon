#version 430

layout (location = 0) out vec4 o_Color;

layout (location = 0) in vec2 v_TexCoord;

layout (binding = 0) uniform sampler2D u_Texture;

void main()
{
	const float gamma     = 2.2;
	const float pureWhite = 1.0;

	// Tonemapping

	vec3 color = texture(u_Texture, v_TexCoord).rgb;

	// Reinhard tonemapping operator.
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale color by ratio of average luminances.
	vec3 mappedColor = (mappedLuminance / luminance) * color;

	// Gamma correction.
	o_Color = vec4(pow(mappedColor, vec3(1.0 / gamma)), 1.0);
}