#include "neopch.h"

#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "VulkanVertexBuffer.h"

namespace Neon
{
	VulkanVertexBuffer::VulkanVertexBuffer(const void* data, uint32 size, const VertexBufferLayout& layout)
		: VertexBuffer(size, layout)
	{
		// TODO: Create device local buffer
		const auto& device = VulkanContext::GetDevice();

		VulkanAllocator allocator(device, "VertexBuffer");

		allocator.AllocateBuffer(m_Buffer, size, vk::BufferUsageFlagBits::eVertexBuffer,
								 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		allocator.UpdateBuffer(m_Buffer, data);
	}
} // namespace Neon
