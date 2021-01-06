#pragma once

#include "Renderer/VertexBuffer.h"
#include "Vulkan.h"
#include "VulkanAllocator.h"

namespace Neon
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(void* data, uint32 size, const VertexBufferLayout& layout);
		~VulkanVertexBuffer() = default;

		vk::Buffer GetHandle() const
		{
			return m_Buffer.Handle.get();
		}

	private:
		VulkanBuffer m_Buffer;
	};
} // namespace Neon
