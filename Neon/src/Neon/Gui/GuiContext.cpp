#include "neopch.h"

#include "GuiContext.h"
#include "Neon/Platform/Vulkan/VulkanGuiContext.h"
#include "Neon/Renderer/RendererAPI.h"

namespace Neon
{
	SharedRef<GuiContext> GuiContext::Create()
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanGuiContext>::Create();
		}

		NEO_CORE_ASSERT(false, "Unknown RendererAPI is selected");
		return nullptr;
	}
} // namespace Neon
