#version 450 core

layout (binding = 0, rgba32f) uniform writeonly image2D u_Displacement[3];

layout (binding = 1, rgba32f) uniform readonly image2D u_ButterflyResult[3];

layout (std140, binding = 2) uniform PropertiesUBO
{
	int u_N;
};

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{	
	for (uint i = 0; i < 3; i++)
	{
		float res = imageLoad(u_ButterflyResult[i], ivec2(gl_GlobalInvocationID)).r / float(u_N * u_N);
		if (mod(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y, 2) > 0)
		{
			res *= -1;
		}
		imageStore(u_Displacement[i], ivec2(gl_GlobalInvocationID), vec4(res, res, res, 1));
	}
}
