#version 450 core

layout (binding = 0, rgba32f) uniform writeonly image2D u_Displacement;

layout (binding = 1, rgba32f) uniform readonly image2D u_PingPong0;
layout (binding = 2, rgba32f) uniform readonly image2D u_PingPong1;

layout (std140, binding = 3) uniform UBO
{
	int u_PingPong;
};

int N = 256;

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	
	float perms[] = {1.0, -1.0};
	int index = int(mod((int(x.x + x.y)), 2));
	float perm = perms[index];
	
	if (u_PingPong == 0)
	{
		float h = imageLoad(u_PingPong0, x).r;
		imageStore(u_Displacement, x, vec4(perm * (h / float(N * N)), perm * (h / float(N * N)), perm * (h / float(N * N)), 1));
	}
	else if (u_PingPong == 1)
	{
		float h = imageLoad(u_PingPong1, x).r;
		imageStore(u_Displacement, x, vec4(perm * (h / float(N * N)), perm * (h / float(N * N)), perm * (h / float(N * N)), 1));
	}
}
