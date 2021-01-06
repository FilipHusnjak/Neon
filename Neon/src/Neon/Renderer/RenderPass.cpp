#include "neopch.h"

#include "Platform/Vulkan/VulkanRenderPass.h"
#include "RenderPass.h"
#include "Renderer.h"

namespace Neon
{
	RenderPass::RenderPass(const RenderPassSpecification& spec)
		: m_Specification(spec)
	{
	}

	SharedRef<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
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
				return SharedRef<VulkanRenderPass>::Create(spec);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}
} // namespace Neon
