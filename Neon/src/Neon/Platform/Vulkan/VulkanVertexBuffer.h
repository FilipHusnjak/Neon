#pragma once

#include "Neon/Platform/Vulkan/Vulkan.h"
#include "Neon/Platform/Vulkan/VulkanAllocator.h"
#include "Neon/Renderer/VertexBuffer.h"

namespace Neon
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const void* data, uint32 size, const VertexBufferLayout& layout);
		~VulkanVertexBuffer() = default;

		void* GetHandle() const override
		{
			return m_Buffer.Handle.get();
		}

	private:
		VulkanBuffer m_Buffer;
	};
} // namespace Neon
