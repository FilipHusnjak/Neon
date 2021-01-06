#include "neopch.h"

#include "Framebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Renderer.h"

namespace Neon
{
	Framebuffer::Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
	}

	SharedRef<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		SharedRef<Framebuffer> result = nullptr;
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				NEO_CORE_ASSERT(false, "Renderer API not selected!");
				return nullptr;
			}
			case RendererAPI::API::Vulkan:
			{
				result = SharedRef<VulkanFramebuffer>::Create(spec);
			}
		}
		NEO_CORE_ASSERT(result, "Renderer API not selected!");

		FramebufferPool::Get().Add(result.Ptr());
		return result;
	}

	void FramebufferPool::Add(Framebuffer* framebuffer)
	{
		m_Framebuffers.push_back(framebuffer);
	}

	FramebufferPool* FramebufferPool::s_Instance = new FramebufferPool;

} // namespace Neon
