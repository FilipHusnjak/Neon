#include "neopch.h"

#include "Platform/Vulkan/VulkanShader.h"
#include "Renderer.h"
#include "Shader.h"

namespace Neon
{
	Shader::Shader(const ShaderSpecification& specification)
		: m_VertexBufferLayout(specification.VBLayout)
	{
	}

	SharedRef<Shader> Shader::Create(const ShaderSpecification& shaderSpecification,
									 const std::unordered_map<ShaderType, std::string>& shaderPaths)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				NEO_CORE_ASSERT(false, "Renderer API not selected!");
				return nullptr;
			}
			case RendererAPI::API::Vulkan:
			{
				return SharedRef<VulkanShader>::Create(shaderSpecification, shaderPaths);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}
} // namespace Neon
