#pragma once

#include "glm/glm.hpp"

namespace Neon
{
	enum class ShaderType
	{
		Vertex,
		Fragment
	};

	enum class UniformType
	{
		Sampler,
		CombinedImageSampler,
		SampledImage,
		StorageImage,
		UniformTexelBuffer,
		StorageTexelBuffer,
		UniformBuffer,
		StorageBuffer,
		UniformBufferDynamic,
		StorageBufferDynamic,
		InputAttachment,
		InlineUniformBlockEXT,
		AccelerationStructureNV
	};

	enum ShaderStageFlag
	{
		Vertex = BIT(0),
		Fragment = BIT(1)
	};

	struct UniformBinding
	{
		uint32 Binding;
		UniformType Type;
		uint32 Count;
		uint32 Size;
		uint32 ShaderStageFlags;
	};

	class Shader : public RefCounted
	{
	public:
		virtual ~Shader() = default;

		virtual void LoadShader(const std::string& path, ShaderType type) = 0;

		virtual void SetUniformBuffer(uint32 binding, uint32 index, const void* data) = 0;

		static SharedRef<Shader> Create(const std::vector<UniformBinding>& bindings);
	};
} // namespace Neon
