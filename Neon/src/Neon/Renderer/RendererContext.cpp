#include "neopch.h"

#include "Neon/Core/Application.h"
#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "Neon/Renderer/Renderer.h"
#include "RendererContext.h"

namespace Neon
{
	SharedRef<RendererContext> RendererContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				NEO_CORE_ASSERT(false, "RendererAPI is not selected");
				return nullptr;
			}
			case RendererAPI::API::Vulkan:
			{
				return SharedRef<VulkanContext>::Create(static_cast<GLFWwindow*>(window));
			}
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI is selected");
		return nullptr;
	}

	void RendererContext::SafeDeleteResource(const StaleResourceWrapper& staleResourceWrapper)
	{
		GetPrimaryRenderCommandBuffer()->SafeDestroyResource(staleResourceWrapper);
	}

	SharedRef<RendererContext> RendererContext::Get()
	{
		return Application::Get().GetWindow().GetRenderContext();
	}

} // namespace Neon
