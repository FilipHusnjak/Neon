#version 450 core

layout (binding = 0, rgba32f) readonly uniform image2D u_DerivativesXX;
layout (binding = 1, rgba32f) readonly uniform image2D u_DerivativesZZ;
layout (binding = 2, rgba32f) readonly uniform image2D u_DerivativesXZ;

layout (binding = 3, rgba32f) writeonly uniform image2D u_Result;

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	float dxdx = imageLoad(u_DerivativesXX, ivec2(gl_GlobalInvocationID.xy)).r;
    float dzdz = imageLoad(u_DerivativesZZ, ivec2(gl_GlobalInvocationID.xy)).r;
    float dxdz = imageLoad(u_DerivativesXZ, ivec2(gl_GlobalInvocationID.xy)).r;

    float j = (1 + dxdx) * (1 + dzdz) - dxdz * dxdz;
    imageStore(u_Result, ivec2(gl_GlobalInvocationID.xy), vec4(j, j, j, 1.0));
}