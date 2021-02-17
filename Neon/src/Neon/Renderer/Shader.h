#pragma once

#include "Neon/Renderer/Texture.h"
#include "Neon/Renderer/VertexBuffer.h"

#include <glm/glm.hpp>
#include <shaderc/shaderc.hpp>

namespace Neon
{
	enum class ShaderType
	{
		Vertex,
		Fragment,
		Compute
	};

	static shaderc_shader_kind ShaderTypeToShadercShaderType(ShaderType shaderType)
	{
		switch (shaderType)
		{
			case ShaderType::Vertex:
				return shaderc_vertex_shader;
			case ShaderType::Fragment:
				return shaderc_fragment_shader;
			case ShaderType::Compute:
				return shaderc_compute_shader;
			default:
				NEO_CORE_ASSERT(false, "Uknown shader type!");
				return shaderc_glsl_infer_from_source;
		}
	}

	enum class UniformType
	{
		None = 0,
		Bool,
		Int,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Mat3,
		Mat4
	};

	struct ShaderSpecification
	{
		VertexBufferLayout VBLayout;
		std::unordered_map<std::string, uint32> ShaderVariableCounts;
		std::unordered_map<ShaderType, std::string> ShaderPaths;
	};

	class Shader : public RefCounted
	{
	public:
		static SharedRef<Shader> Create(const ShaderSpecification& shaderSpecification);

	public:
		Shader(const ShaderSpecification& specification);
		virtual ~Shader() = default;

		virtual void Reload() = 0;

		virtual void SetUniformBuffer(const std::string& name, uint32 index, const void* data, uint32 size = 0) = 0;
		virtual void SetStorageBuffer(const std::string& name, const void* data, uint32 size = 0) = 0;
		virtual void SetPushConstant(const std::string& name, const void* data, uint32 size = 0) = 0;
		virtual void SetTexture2D(const std::string& name, uint32 index, const SharedRef<Texture2D>& texture, uint32 mipLevel) = 0;
		virtual void SetTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& texture,
									uint32 mipLevel) = 0;
		virtual void SetStorageTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& texture,
										   uint32 mipLevel) = 0;

		virtual SharedRef<Texture2D> GetTexture2D(const std::string& name, uint32 index) const = 0;
		virtual SharedRef<TextureCube> GetTextureCube(const std::string& name, uint32 index) const = 0;

		const VertexBufferLayout& GetVertexBufferLayout() const
		{
			return m_Specification.VBLayout;
		}

		const ShaderSpecification& GetShaderSpecification() const
		{
			return m_Specification;
		}

	protected:
		ShaderSpecification m_Specification;
	};
} // namespace Neon
