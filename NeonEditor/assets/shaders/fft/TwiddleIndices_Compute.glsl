#version 450 core

#define PI 3.1415926535897932384626433832795

layout (binding = 0, rgba32f) writeonly uniform image2D u_TwiddleIndices;

layout (std430, binding = 1) buffer BitReversedUBO
{
	uint BitsReversed[];
};

layout (std140, binding = 2) uniform PropertiesUBO
{
	uint u_N;
};

struct Complex
{	
	float Real;
	float Im;
};

layout (local_size_x = 1, local_size_y = 32, local_size_z = 1) in;
void main()
{
	uvec2 texCoord = gl_GlobalInvocationID.xy;
	float k = mod(texCoord.y * (float(u_N) / pow(2, texCoord.x + 1)), u_N);
	Complex twiddle = Complex(cos(2.0 * PI * k / float(u_N)), sin(2.0 * PI * k / float(u_N)));
	
	uint butterflyspan = int(pow(2, texCoord.x));

	uvec2 butterflyPair = uvec2(texCoord.y, texCoord.y + butterflyspan);
	// Top butterfly wing
	if (mod(texCoord.y, pow(2, texCoord.x + 1)) >= pow(2, texCoord.x))
	{
		butterflyPair -= uvec2(butterflyspan);
	}

	// First stage, bit reversed indices
	if (texCoord.x == 0)
	{
		butterflyPair = uvec2(BitsReversed[butterflyPair.x], BitsReversed[butterflyPair.y]);
	}

	imageStore(u_TwiddleIndices, ivec2(gl_GlobalInvocationID.xy), vec4(twiddle.Real, twiddle.Im, butterflyPair));
}
