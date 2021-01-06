#include "neopch.h"

#include "Platform/Vulkan/VulkanShader.h"
#include "Renderer.h"
#include "Shader.h"

namespace Neon
{
	SharedRef<Shader> Shader::Create(const std::vector<UniformBinding>& bindings)
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
				return SharedRef<VulkanShader>::Create(bindings);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}
} // namespace Neon
