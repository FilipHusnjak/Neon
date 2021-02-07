#include "neopch.h"

#include "CommandBuffer.h"
#include "Neon/Platform/Vulkan/VulkanCommandBuffer.h"
#include "Neon/Renderer/RendererAPI.h"

namespace Neon
{

	SharedRef<CommandBuffer> CommandBuffer::Create(const SharedRef<CommandPool>& commandPool)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanCommandBuffer>::Create(commandPool);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	CommandBuffer::CommandBuffer(const SharedRef<CommandPool>& commandPool)
		: m_Pool(commandPool)
	{
	}

	SharedRef<CommandPool> CommandPool::Create(CommandBufferType type)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanCommandPool>::Create(type);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	CommandPool::CommandPool(CommandBufferType type)
		: m_Type(type)
	{
	}

} // namespace Neon
