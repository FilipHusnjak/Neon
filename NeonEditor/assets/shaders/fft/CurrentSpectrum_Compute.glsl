#version 450 core

#define PI 3.1415926535897932384626433832795

layout (binding = 0, rgba32f) writeonly uniform image2D u_HktDy; // height displacement
layout (binding = 1, rgba32f) writeonly uniform image2D u_HktDx; // x displacement
layout (binding = 2, rgba32f) writeonly uniform image2D u_HktDz; // z displacement
layout (binding = 3, rgba32f) writeonly uniform image2D u_HktDyDx; // dHktDy / dx
layout (binding = 4, rgba32f) writeonly uniform image2D u_HktDyDz; // dHktDy / dz
layout (binding = 5, rgba32f) writeonly uniform image2D u_HktDxDx; // dHktDx / dx
layout (binding = 6, rgba32f) writeonly uniform image2D u_HktDzDz; // dHktDz / dz
layout (binding = 7, rgba32f) writeonly uniform image2D u_HktDxDz; // dHktDx / dz

layout (binding = 8, rgba32f) readonly uniform image2D u_H0k;

layout (std140, binding = 9) uniform PropertiesUBO
{
	uint u_N;
	float u_L;
};

layout (std140, binding = 10) uniform TimeUBO
{
    float u_Time;
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

Complex Conjugate(Complex c)
{
	return Complex(c.Real, -c.Im);
}

Complex FromVec2(vec2 vec)
{
	return Complex(vec.x, vec.y);
}

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	vec2 v = ivec2(gl_GlobalInvocationID.xy) - float(u_N) / 2.0;
	
	vec2 k = 2.0 * PI / u_L * v;
	
	float mag = max(0.0001, length(k));
	
	// Dispersion relation for deep water waves
	float wt = sqrt(9.81 * mag) * u_Time;
	
	// Euler formula
	Complex amp = FromVec2(imageLoad(u_H0k, ivec2(gl_GlobalInvocationID.xy)).rg);
	Complex ampConj = Conjugate(FromVec2(imageLoad(u_H0k, int(u_N) - ivec2(gl_GlobalInvocationID.xy)).rg));

	Complex exponent = Complex(cos(wt), sin(wt));
	
	// dy
	Complex hktDy = Add(Mul(amp, exponent), Mul(ampConj, Conjugate(exponent)));
	imageStore(u_HktDy, ivec2(gl_GlobalInvocationID.xy), vec4(hktDy.Real, hktDy.Im, 0, 1));
	
	// dx
	Complex hktDx = Mul(Complex(0.0, k.x / mag), hktDy);
	imageStore(u_HktDx, ivec2(gl_GlobalInvocationID.xy), vec4(hktDx.Real, hktDx.Im, 0, 1));
	
	// dz
	Complex hktDz = Mul(Complex(0.0, k.y / mag), hktDy);
	imageStore(u_HktDz, ivec2(gl_GlobalInvocationID.xy), vec4(hktDz.Real, hktDz.Im, 0, 1));

	// dHktDy / dx
	Complex hktDyDx = Mul(Complex(0.0, k.x), hktDy);
	imageStore(u_HktDyDx, ivec2(gl_GlobalInvocationID.xy), vec4(hktDyDx.Real, hktDyDx.Im, 0, 1));

	// dHktDy / dz
	Complex hktDyDz = Mul(Complex(0.0, k.y), hktDy);
	imageStore(u_HktDyDz, ivec2(gl_GlobalInvocationID.xy), vec4(hktDyDz.Real, hktDyDz.Im, 0, 1));

	// dHktDx / dx
	Complex hktDxDx = Mul(Complex(0.0, k.x), hktDx);
	imageStore(u_HktDxDx, ivec2(gl_GlobalInvocationID.xy), vec4(hktDxDx.Real, hktDxDx.Im, 0, 1));

	// dHktDz / dz
	Complex hktDzDz = Mul(Complex(0.0, k.y), hktDz);
	imageStore(u_HktDzDz, ivec2(gl_GlobalInvocationID.xy), vec4(hktDzDz.Real, hktDzDz.Im, 0, 1));

	// dHktDx / dz
	Complex hktDxDz = Mul(Complex(0.0, k.y), hktDx);
	imageStore(u_HktDxDz, ivec2(gl_GlobalInvocationID.xy), vec4(hktDxDz.Real, hktDxDz.Im, 0, 1));
}
