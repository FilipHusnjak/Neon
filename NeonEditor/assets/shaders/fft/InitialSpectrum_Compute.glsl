#version 450 core

#define M_PI 3.1415926535897932384626433832795

layout (binding = 0, rgba32f) writeonly uniform image2D u_H0k;
layout (binding = 1, rgba32f) writeonly uniform image2D u_H0minusk;

layout (binding = 2) uniform sampler2D u_Noise0;
layout (binding = 3) uniform sampler2D u_Noise1;
layout (binding = 4) uniform sampler2D u_Noise2;
layout (binding = 5) uniform sampler2D u_Noise3;

int N = 256;
int L = 1000;
float A = 20;
vec2 w = vec2(1, 0);
float windspeed = 26;

const float g = 9.81;

// Box-Muller-Method
vec4 GaussRnd()
{
	vec2 texCoord = vec2(gl_GlobalInvocationID.xy) / float(N);
	
	float noise00 = clamp(texture(u_Noise0, texCoord).r, 0.001, 1.0);
	float noise01 = clamp(texture(u_Noise1, texCoord).r, 0.001, 1.0);
	float noise02 = clamp(texture(u_Noise2, texCoord).r, 0.001, 1.0);
	float noise03 = clamp(texture(u_Noise3, texCoord).r, 0.001, 1.0);
	
	float u0 = 2.0 * M_PI * noise00;
	float v0 = sqrt(-2.0 * log(noise01));
	float u1 = 2.0 * M_PI * noise02;
	float v1 = sqrt(-2.0 * log(noise03));
	
	vec4 rnd = vec4(v0 * cos(u0), v0 * sin(u0), v1 * cos(u1), v1 * sin(u1));
	
	return rnd;
}

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
	vec2 x = vec2(gl_GlobalInvocationID.xy);
	
	vec2 k = 2.0 * M_PI / L * x;

	float L_ = (windspeed * windspeed) / g;
	float mag = max(0.0001, length(k));
	float magSq = mag * mag;
	
	//sqrt(Ph(k))/sqrt(2)
	float h0k = clamp(sqrt((A / (magSq * magSq)) * pow(dot(normalize(k), normalize(w)), 2) * 
				exp(-(1.0 / (magSq * L_ * L_))) * exp(-magSq * pow(L / 2000.0, 2.0))) / sqrt(2.0), -4000.0, 4000.0);
	
	//sqrt(Ph(-k))/sqrt(2)
	float h0minusk = clamp(sqrt((A / (magSq * magSq)) * pow(dot(normalize(-k), normalize(w)), 2) * 
					 exp(-(1.0 / (magSq * L_ * L_))) * exp(-magSq * pow(L / 2000.0, 2.0))) / sqrt(2.0), -4000.0, 4000.0);
	
	vec4 rnd = GaussRnd();
	
	imageStore(u_H0k, ivec2(gl_GlobalInvocationID.x, 255 - gl_GlobalInvocationID.y), vec4(rnd.xy * h0k, 0, 1));
	imageStore(u_H0minusk, ivec2(gl_GlobalInvocationID.xy), vec4(rnd.zw * h0minusk, 0, 1));
}
