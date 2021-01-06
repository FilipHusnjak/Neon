#include "neopch.h"

#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Renderer.h"
#include "VertexBuffer.h"

namespace Neon
{
	VertexBuffer::VertexBuffer(uint32 size, const VertexBufferLayout& layout)
		: m_Size(size)
		, m_Layout(layout)
	{
	}

	SharedRef<VertexBuffer> VertexBuffer::Create(void* data, uint32 size, const VertexBufferLayout& layout)
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
				return SharedRef<VulkanVertexBuffer>::Create(data, size, layout);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}
} // namespace Neon
