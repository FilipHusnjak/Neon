#include "neopch.h"

#include "Platform/Vulkan/VulkanShader.h"
#include "Renderer.h"
#include "Shader.h"

namespace Neon
{
	Shader::Shader(const ShaderSpecification& specification)
		: m_Specification(specification)
	{
	}

	SharedRef<Shader> Shader::Create(const ShaderSpecification& shaderSpecification)
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
				return SharedRef<VulkanShader>::Create(shaderSpecification);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}
} // namespace Neon
