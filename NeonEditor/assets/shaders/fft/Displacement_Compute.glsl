#version 450 core

layout (binding = 0, rgba32f) uniform writeonly image2D u_Displacement;

layout (binding = 1, rgba32f) uniform readonly image2D u_PingPong[2];

layout (std140, binding = 3) uniform UBO
{
	int u_PingPongIndex;
};

int N = 256;

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{	
	float res = imageLoad(u_PingPong[u_PingPongIndex], ivec2(gl_GlobalInvocationID)).r / float(N * N);
	if (mod(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y, 2) > 0)
	{
		res *= -1;
	}
	imageStore(u_Displacement, ivec2(gl_GlobalInvocationID), vec4(res, res, res, 1));
}
