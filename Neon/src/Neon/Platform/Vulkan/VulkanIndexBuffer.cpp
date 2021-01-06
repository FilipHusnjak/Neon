#include "neopch.h"

#include "VulkanIndexBuffer.h"
#include "VulkanContext.h"

namespace Neon
{
	VulkanIndexBuffer::VulkanIndexBuffer(void* data, uint32 size)
		: IndexBuffer(size)
	{
		// TODO: Create device local buffer
		const auto& device = VulkanContext::GetDevice();

		VulkanAllocator allocator(device, "IndexBuffer");

		allocator.AllocateBuffer(m_Buffer, size, vk::BufferUsageFlagBits::eIndexBuffer,
								 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		allocator.UpdateBuffer(m_Buffer, data);
	}
} // namespace Neon
