#include "neopch.h"

#include "ImGuiLayer.h"
#include "Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanImGuiLayer.h"

namespace Neon
{
	ImGuiLayer* ImGuiLayer::Create()
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return new VulkanImGuiLayer();
		}

		NEO_CORE_ASSERT(false, "Unknown RendererAPI is selected");
		return nullptr;
	}
} // namespace Neon
