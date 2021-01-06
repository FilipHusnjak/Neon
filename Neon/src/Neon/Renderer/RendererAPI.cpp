#include "neopch.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"
#include "RendererAPI.h"

namespace Neon
{
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

	UniqueRef<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
			case RendererAPI::API::None:
			{
				NEO_CORE_ASSERT(false, "RendererAPI is not selected");
				return nullptr;
			}
			case RendererAPI::API::Vulkan:
			{
				return CreateUnique<VulkanRendererAPI>();
			}
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI is selected");
		return nullptr;
	}
} // namespace Neon
