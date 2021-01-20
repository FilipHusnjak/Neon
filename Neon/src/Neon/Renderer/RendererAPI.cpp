#include "neopch.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"
#include "RendererAPI.h"

namespace Neon
{
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

	void RendererAPI::Init()
	{
		m_QuadVertexBuffer = VertexBuffer::Create(m_QuadVertices.data(), static_cast<uint32>(m_QuadVertices.size()) * sizeof(m_QuadVertices[0]),
												  VertexBufferLayout({ShaderDataType::Float2}));
		m_QuadIndexBuffer = IndexBuffer::Create(m_QuadIndices.data(), static_cast<uint32>(m_QuadIndices.size()) * sizeof(m_QuadIndices[0]));
	}

	void RendererAPI::Shutdown()
	{
		m_QuadVertexBuffer.Reset();
		m_QuadIndexBuffer.Reset();
	}

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
