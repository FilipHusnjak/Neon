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
		Fragment
	};

	static shaderc_shader_kind ShaderTypeToShadercShaderType(ShaderType shaderType)
	{
		switch (shaderType)
		{
			case ShaderType::Vertex:
				return shaderc_vertex_shader;
			case ShaderType::Fragment:
				return shaderc_fragment_shader;
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
	};

	class Shader : public RefCounted
	{
	public:
		Shader(const ShaderSpecification& specification);
		virtual ~Shader() = default;

		virtual void Reload() = 0;

		virtual void SetUniformBuffer(uint32 binding, uint32 index, const void* data) = 0;
		virtual void SetStorageBuffer(uint32 binding, const void* data) = 0;
		virtual void SetTexture2D(uint32 binding, uint32 index, const SharedRef<Texture2D>& texture) = 0;
		virtual void SetTextureCube(uint32 binding, uint32 index, const SharedRef<TextureCube>& texture) = 0;

		const VertexBufferLayout& GetVertexBufferLayout() const
		{
			return m_VertexBufferLayout;
		}

		static SharedRef<Shader> Create(const ShaderSpecification& shaderSpecification,
										const std::unordered_map<ShaderType, std::string>& shaderPaths);

	protected:
		VertexBufferLayout m_VertexBufferLayout;
	};
} // namespace Neon
