#version 450 core

layout (binding = 0, rgba32f) uniform writeonly image2D u_Result[8];

layout (binding = 1, rgba32f) uniform readonly image2D u_ButterflyResult[8];

layout (std140, binding = 2) uniform PropertiesUBO
{
	int u_N;
};

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	int sign = 1;
	if (mod(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y, 2) > 0)
	{
		sign *= -1;
	}

	for (uint i = 0; i < 8; i++)
	{
		float res = sign * imageLoad(u_ButterflyResult[i], ivec2(gl_GlobalInvocationID)).r;
		imageStore(u_Result[i], ivec2(gl_GlobalInvocationID), vec4(res, res, res, 1));
	}
}
