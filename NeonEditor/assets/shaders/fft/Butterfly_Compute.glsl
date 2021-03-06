#version 450 core

layout (binding = 0, rgba32f) readonly uniform image2D u_TwiddleIndices;

layout (binding = 1, rgba32f) uniform image2D u_PingPong[2];

layout (std140, binding = 2) uniform PropertiesUBO
{
	uint u_Stage;
	uint u_PingPongIndex;
	uint u_Direction;
};

struct Complex
{
	float Real;
	float Im;
};

Complex Mul(Complex c0, Complex c1)
{
	Complex c;
	c.Real = c0.Real * c1.Real - c0.Im * c1.Im;
	c.Im = c0.Real * c1.Im + c0.Im * c1.Real;
	return c;
}

Complex Add(Complex c0, Complex c1)
{
	Complex c;
	c.Real = c0.Real + c1.Real;
	c.Im = c0.Im + c1.Im;
	return c;
}

Complex FromVec2(vec2 vec)
{
	return Complex(vec.x, vec.y);
}

void Horizontal()
{
	vec4 data = imageLoad(u_TwiddleIndices, ivec2(u_Stage, gl_GlobalInvocationID.x)).rgba;
	Complex p = FromVec2(imageLoad(u_PingPong[u_PingPongIndex], ivec2(data.z, gl_GlobalInvocationID.y)).rg);
	Complex q = FromVec2(imageLoad(u_PingPong[u_PingPongIndex], ivec2(data.w, gl_GlobalInvocationID.y)).rg);
	Complex w = Complex(data.x, data.y);
		
	// Butterfly operation
	Complex result = Add(p, Mul(w, q));
		
	imageStore(u_PingPong[int(mod(u_PingPongIndex + 1, 2))], ivec2(gl_GlobalInvocationID.xy), vec4(result.Real, result.Im, 0, 1));
}

void Vertical()
{
	vec4 data = imageLoad(u_TwiddleIndices, ivec2(u_Stage, gl_GlobalInvocationID.y)).rgba;
	Complex p = FromVec2(imageLoad(u_PingPong[u_PingPongIndex], ivec2(gl_GlobalInvocationID.x, data.z)).rg);
	Complex q = FromVec2(imageLoad(u_PingPong[u_PingPongIndex], ivec2(gl_GlobalInvocationID.x, data.w)).rg);
	Complex w = Complex(data.x, data.y);
		
	// Butterfly operation
	Complex result = Add(p, Mul(w, q));
		
	imageStore(u_PingPong[int(mod(u_PingPongIndex + 1, 2))], ivec2(gl_GlobalInvocationID.xy), vec4(result.Real, result.Im, 0, 1));
}

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	if (u_Direction == 0)
	{
		Horizontal();
	}
	else
	{
		Vertical();
	}
}
