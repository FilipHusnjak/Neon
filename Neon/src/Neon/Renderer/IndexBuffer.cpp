#include "neopch.h"

#include "IndexBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Renderer.h"

namespace Neon
{
	IndexBuffer::IndexBuffer(uint32 size)
		: m_Size(size)
	{
	}

	SharedRef<IndexBuffer> IndexBuffer::Create(void* data, uint32 size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				NEO_CORE_ASSERT(false, "Renderer API not selected!");
				return nullptr;
			}
			case RendererAPI::API::Vulkan:
			{
				return SharedRef<VulkanIndexBuffer>::Create(data, size);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}

} // namespace Neon
