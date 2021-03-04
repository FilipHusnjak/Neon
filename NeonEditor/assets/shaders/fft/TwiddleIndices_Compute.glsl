#version 450 core

#define M_PI 3.1415926535897932384626433832795

layout (binding = 0, rgba32f) writeonly uniform image2D u_TwiddleIndices;

layout (std430, binding = 1) buffer BitReversedUBO
{
	int BitsReversed[];
};

struct Complex
{	
	float Real;
	float Im;
};

int N = 256;

layout (local_size_x = 1, local_size_y = 32, local_size_z = 1) in;
void main()
{
	vec2 x = gl_GlobalInvocationID.xy;
	float k = mod(x.y * (float(N) / pow(2, x.x + 1)), N);
	Complex twiddle = Complex(cos(2.0 * M_PI * k / float(N)), sin(2.0 * M_PI * k / float(N)));
	
	int butterflyspan = int(pow(2, x.x));
	
	int butterflywing;
	
	if (mod(x.y, pow(2, x.x + 1)) < pow(2, x.x))
	{
		butterflywing = 1;
	}
	else
	{
		butterflywing = 0;
	}

	// first stage, bit reversed indices
	if (x.x == 0)
	{
		// top butterfly wing
		if (butterflywing == 1)
		{
			imageStore(u_TwiddleIndices, ivec2(x), vec4(twiddle.Real, twiddle.Im, BitsReversed[int(x.y)], BitsReversed[int(x.y + 1)]));
		}
		// bot butterfly wing
		else	
		{
			imageStore(u_TwiddleIndices, ivec2(x), vec4(twiddle.Real, twiddle.Im, BitsReversed[int(x.y - 1)], BitsReversed[int(x.y)]));
		}
	}
	// second to log2(N) stage
	else
	{
		// top butterfly wing
		if (butterflywing == 1)
		{
			imageStore(u_TwiddleIndices, ivec2(x), vec4(twiddle.Real, twiddle.Im, x.y, x.y + butterflyspan));
		}
		// bot butterfly wing
		else
		{
			imageStore(u_TwiddleIndices, ivec2(x), vec4(twiddle.Real, twiddle.Im, x.y - butterflyspan, x.y));
		}
	}
}
