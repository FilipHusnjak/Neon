#version 450 core

#define PI 3.1415926535897932384626433832795

layout (binding = 0, rgba32f) writeonly uniform image2D u_HktDy; // height displacement
layout (binding = 1, rgba32f) writeonly uniform image2D u_HktDx; // x displacement
layout (binding = 2, rgba32f) writeonly uniform image2D u_HktDz; // z displacement

layout (binding = 3, rgba32f) readonly uniform image2D u_H0k;
layout (binding = 4, rgba32f) readonly uniform image2D u_H0minusk;

int N = 256;
int L = 1000;

layout (std140, binding = 5) uniform TimeUBO
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
	vec2 v = ivec2(gl_GlobalInvocationID.xy) - float(N) / 2.0;
	
	vec2 k = 2.0 * PI / L * v;
	
	float mag = max(0.0001, length(k));
	
	// Dispersion relation for water waves
	float w = sqrt(9.81 * mag);
	
	Complex fourierAmp = FromVec2(imageLoad(u_H0k, ivec2(gl_GlobalInvocationID.xy)).rg);
	Complex fourierAmpConj = Conjugate(FromVec2(imageLoad(u_H0minusk, ivec2(gl_GlobalInvocationID.xy)).rg));
	
	// euler formula
	Complex exp_iwt = Complex(cos(w * u_Time), sin(w * u_Time));
	Complex exp_iwt_inv = Conjugate(exp_iwt);
	
	// dy
	Complex hktDy = Add(Mul(fourierAmp, exp_iwt), Mul(fourierAmpConj, exp_iwt_inv));
	imageStore(u_HktDy, ivec2(gl_GlobalInvocationID.xy), vec4(hktDy.Real, hktDy.Im, 0, 1));
	
	// dx
	Complex hktDx = Mul(Complex(0.0, k.x / mag), hktDy);
	imageStore(u_HktDx, ivec2(gl_GlobalInvocationID.xy), vec4(hktDx.Real, hktDx.Im, 0, 1));
	
	// dz
	Complex hktDz = Mul(Complex(0.0, k.y / mag), hktDy);
	imageStore(u_HktDz, ivec2(gl_GlobalInvocationID.xy), vec4(hktDz.Real, hktDz.Im, 0, 1));
}
