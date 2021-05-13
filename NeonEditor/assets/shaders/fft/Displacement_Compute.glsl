#version 450 core

layout (binding = 0, rgba32f) uniform writeonly image2D u_Displacement;
layout (binding = 1, rgba32f) uniform readonly image2D u_ButterflyResult[3];

layout (std140, binding = 2) uniform PropertiesUBO
{
	int u_N;
};

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	float sign = 1;
	if (mod(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y, 2) > 0)
	{
		sign *= -1;
	}
	imageStore(u_Displacement, ivec2(gl_GlobalInvocationID), vec4(sign * imageLoad(u_ButterflyResult[0], ivec2(gl_GlobalInvocationID)).r, 
																  sign * imageLoad(u_ButterflyResult[1], ivec2(gl_GlobalInvocationID)).r, 
															      sign * imageLoad(u_ButterflyResult[2], ivec2(gl_GlobalInvocationID)).r, 1));
}
