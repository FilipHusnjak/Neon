#pragma once

#include "Renderer/IndexBuffer.h"
#include "Vulkan.h"
#include "VulkanAllocator.h"

namespace Neon
{
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const void* data, uint32 size);
		~VulkanIndexBuffer() = default;

		void* GetHandle() const override
		{
			return m_Buffer.Handle.get();
		}

	private:
		VulkanBuffer m_Buffer;
	};
} // namespace Neon
