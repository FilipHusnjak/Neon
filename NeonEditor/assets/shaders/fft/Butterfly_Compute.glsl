#version 450 core
 
#define M_PI 3.1415926535897932384626433832795

layout (binding = 0, rgba32f) readonly uniform image2D u_TwiddleIndices;

layout (binding = 1, rgba32f) uniform image2D u_PingPong0;
layout (binding = 2, rgba32f) uniform image2D u_PingPong1;

layout (std140, binding = 3) uniform UBO
{
	int u_Stage;
	int u_PingPong;
	int u_Direction;
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

void HorizontalButterflies()
{
	Complex H;
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	
	if (u_PingPong == 0)
	{
		vec4 data = imageLoad(u_TwiddleIndices, ivec2(u_Stage, x.x)).rgba;
		vec2 p_ = imageLoad(u_PingPong0, ivec2(data.z, x.y)).rg;
		vec2 q_ = imageLoad(u_PingPong0, ivec2(data.w, x.y)).rg;
		vec2 w_ = vec2(data.x, data.y);
		
		Complex p = Complex(p_.x,p_.y);
		Complex q = Complex(q_.x,q_.y);
		Complex w = Complex(w_.x,w_.y);
		
		//Butterfly operation
		H = Add(p, Mul(w, q));
		
		imageStore(u_PingPong1, x, vec4(H.Real, H.Im, 0, 1));
	}
	else if (u_PingPong == 1)
	{
		vec4 data = imageLoad(u_TwiddleIndices, ivec2(u_Stage, x.x)).rgba;
		vec2 p_ = imageLoad(u_PingPong1, ivec2(data.z, x.y)).rg;
		vec2 q_ = imageLoad(u_PingPong1, ivec2(data.w, x.y)).rg;
		vec2 w_ = vec2(data.x, data.y);
		
		Complex p = Complex(p_.x,p_.y);
		Complex q = Complex(q_.x,q_.y);
		Complex w = Complex(w_.x,w_.y);
		
		//Butterfly operation
		H = Add(p, Mul(w, q));
		
		imageStore(u_PingPong0, x, vec4(H.Real, H.Im, 0, 1));
	}
}

void VerticalButterflies()
{
	Complex H;
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	
	if (u_PingPong == 0)
	{
		vec4 data = imageLoad(u_TwiddleIndices, ivec2(u_Stage, x.y)).rgba;
		vec2 p_ = imageLoad(u_PingPong0, ivec2(x.x, data.z)).rg;
		vec2 q_ = imageLoad(u_PingPong0, ivec2(x.x, data.w)).rg;
		vec2 w_ = vec2(data.x, data.y);
		
		Complex p = Complex(p_.x,p_.y);
		Complex q = Complex(q_.x,q_.y);
		Complex w = Complex(w_.x,w_.y);
		
		//Butterfly operation
		H = Add(p, Mul(w, q));
		
		imageStore(u_PingPong1, x, vec4(H.Real, H.Im, 0, 1));
	}
	else if (u_PingPong == 1)
	{
		vec4 data = imageLoad(u_TwiddleIndices, ivec2(u_Stage, x.y)).rgba;
		vec2 p_ = imageLoad(u_PingPong1, ivec2(x.x, data.z)).rg;
		vec2 q_ = imageLoad(u_PingPong1, ivec2(x.x, data.w)).rg;
		vec2 w_ = vec2(data.x, data.y);
		
		Complex p = Complex(p_.x,p_.y);
		Complex q = Complex(q_.x,q_.y);
		Complex w = Complex(w_.x,w_.y);
		
		//Butterfly operation
		H = Add(p, Mul(w, q));
		
		imageStore(u_PingPong0, x, vec4(H.Real, H.Im, 0, 1));
	}
}

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	if (u_Direction == 0)
	{
		HorizontalButterflies();
	}
	else if (u_Direction == 1)
	{
		VerticalButterflies();
	}
}
