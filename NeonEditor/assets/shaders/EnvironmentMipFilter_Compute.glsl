#version 450 core
// Physically Based Rendering

// Pre-filters environment cube map using GGX NDF importance sampling.
// Part of specular IBL split-sum approximation.

const float PI = 3.141592;
const float TwoPI = 2 * PI;
const float Epsilon = 0.00001;

const uint NumSamples = 1024;
const float InvNumSamples = 1.0 / float(NumSamples);

layout (binding = 0) uniform samplerCube u_InputCubemap;
layout (binding = 1, rgba16f) restrict writeonly uniform imageCube o_OutputCubemap;

layout (push_constant) uniform MipLevelPC
{
	float MipCount;
	float MipLevel;
} u_PushConstant;

// Compute Van der Corput radical inverse
// See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Sample i-th point from Hammersley point set of NumSamples points total.
vec2 SampleHammersley(uint i)
{
	return vec2(i * InvNumSamples, RadicalInverse_VdC(i));
}

// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
vec3 SampleGGX(float u1, float u2, float roughness)
{
	float alpha = roughness * roughness;

	float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha * alpha - 1.0) * u2));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	float phi = TwoPI * u1;

	// Convert to Cartesian
	return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NdfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

vec3 GetCubeMapTexCoord()
{
    vec2 st = gl_GlobalInvocationID.xy / vec2(imageSize(o_OutputCubemap));
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if (gl_GlobalInvocationID.z == 0)      ret = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y,  -1.0);
    return normalize(ret);
}

// Compute orthonormal basis for converting from tanget/shading space to world space.
void ComputeBasisVectors(const vec3 N, out vec3 B, out vec3 T)
{
	// Branchless select non-degenerate T.
	T = cross(N, vec3(0.0, 1.0, 0.0));
	T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

	T = normalize(T);
	B = normalize(cross(N, T));
}

// Convert point from T/shading space to world space.
vec3 TangentToWorld(const vec3 v, const vec3 N, const vec3 B, const vec3 T)
{
	return B * v.x + T * v.y + N * v.z;
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	// Make sure we won't write past output when computing higher mipmap levels.
	ivec2 outputSize = imageSize(o_OutputCubemap);
	if(gl_GlobalInvocationID.x >= outputSize.x || gl_GlobalInvocationID.y >= outputSize.y)
	{
		return;
	}
	
	// Solid angle associated with a single cubemap texel at zero mipmap level.
	// This will come in handy for importance sampling below.
	vec2 inputSize = vec2(textureSize(u_InputCubemap, 0));
	float wt = 4.0 * PI / (6.0 * inputSize.x * inputSize.y);
	
	// Approximation: Assume zero viewing angle (isotropic reflections).
	vec3 N = GetCubeMapTexCoord();
	vec3 V = N;
	
	vec3 B, T;
	ComputeBasisVectors(N, B, T);

	vec3 color = vec3(0);
	float weight = 0;

	float deltaRoughness = 1.0 / max((u_PushConstant.MipCount - 1.0), 1.0);
	float roughness = u_PushConstant.MipLevel * deltaRoughness;

	// Convolve environment map using GGX NDF importance sampling.
	// Weight by cosine term since Epic claims it generally improves quality.
	for(uint i = 0; i < NumSamples; i++) {
		vec2 u = SampleHammersley(i);
		vec3 H = TangentToWorld(SampleGGX(u.x, u.y, roughness), N, B, T);

		vec3 L = 2.0 * dot(V, H) * H - V;

		float NdotL = dot(N, L);
		if (NdotL > 0.0)
		{
			// Use Mipmap Filtered Importance Sampling to improve convergence.
			// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

			float NdotH = max(dot(N, H), 0.0);

			// GGX normal distribution function (D term) probability density function.
			// Scaling by 1/4 is due to change of density in terms of Lh to Li (and since N=V, rest of the scaling factor cancels out).
			float pdf = NdfGGX(NdotH, roughness) * 0.25;

			// Solid angle associated with this sample.
			float ws = 1.0 / (NumSamples * pdf);

			// Mip level to sample from.
			float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);

			color  += textureLod(u_InputCubemap, L, mipLevel).rgb * NdotL;
			weight += NdotL;
		}
	}
	color /= weight;

	imageStore(o_OutputCubemap, ivec3(gl_GlobalInvocationID), vec4(color, 1.0));
}
