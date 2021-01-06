#pragma once

#include "Renderer/RenderPass.h"
#include "Vulkan.h"

namespace Neon
{
	static vk::Format FramebufferFormatToVulkanFormat(FramebufferFormat format)
	{
		switch (format)
		{
			case FramebufferFormat::None:
				return vk::Format::eUndefined;
			case FramebufferFormat::RGBA8:
				return vk::Format::eR8G8B8A8Unorm;
			case FramebufferFormat::RGBA16F:
				return vk::Format::eR32G32Sfloat;
		}
		NEO_CORE_ASSERT(false, "Uknown framebuffer format!");
		return vk::Format::eUndefined;
	}

	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(RenderPassSpecification specification);
		~VulkanRenderPass() = default;

		vk::RenderPass GetHandle() const
		{
			return m_Handle.get();
		}

	private:
		vk::UniqueRenderPass m_Handle;
	};
} // namespace Neon
