#version 450 core

layout (binding = 0, rgba32f) uniform writeonly image2D u_Gradients;

layout (binding = 1, rgba32f) uniform readonly image2D u_Displacement;

layout (std140, binding = 2) uniform PropertiesUBO
{
	int u_N;
	float u_L;
};

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	ivec2 loc = ivec2(gl_GlobalInvocationID.xy);
	
	ivec2 left = (loc - ivec2(1, 0)) & (u_N - 1);
	ivec2 right = (loc + ivec2(1, 0)) & (u_N - 1);
	ivec2 bottom = (loc - ivec2(0, 1)) & (u_N - 1);
	ivec2 top = (loc + ivec2(0, 1)) & (u_N - 1);

	vec3 displacementL = imageLoad(u_Displacement, left).xyz;
	vec3 displacementR = imageLoad(u_Displacement, right).xyz;
	vec3 displacementB = imageLoad(u_Displacement, bottom).xyz;
	vec3 displacementT = imageLoad(u_Displacement, top).xyz;

	vec2 gradient = vec2(displacementL.x - displacementR.x, displacementB.x - displacementT.x);

	// Jacobian
	vec2 dDx = (displacementR.yz - displacementL.yz) * float(u_N) / u_L;
	vec2 dDy = (displacementT.yz - displacementB.yz) * float(u_N) / u_L;

	float j = (1.0 + dDx.x) * (1.0 + dDy.y) - dDx.y * dDy.x;

	// NOTE: normals are in tangent space for now
	imageStore(u_Gradients, loc, vec4(gradient.x, u_L / float(u_N), gradient.y, j));
}