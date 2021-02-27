#include "neopch.h"

#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "Neon/Platform/Vulkan/VulkanRenderPass.h"
#include "Neon/Platform/Vulkan/VulkanTexture.h"
#include "Neon/Platform/Vulkan/VulkanFrameBuffer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Neon
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
		: Framebuffer(spec)
	{
		NEO_CORE_ASSERT(m_Specification.Pass);

		Create(spec.Width, spec.Height);
	}

	void VulkanFramebuffer::Resize(uint32 width, uint32 height, bool forceRecreate /*= false*/)
	{
		if (m_Specification.NoResize)
		{
			return;
		}

		if (width == m_Specification.Width && height == m_Specification.Height && !forceRecreate)
		{
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

		Create(width, height);
	}

	void VulkanFramebuffer::Create(uint32 width, uint32 height)
	{
		const auto& device = VulkanContext::GetDevice();

		m_Textures.clear();

		const auto& attachmentDescriptions = m_Specification.Pass->GetSpecification().Attachments;
		std::vector<bool> isInputAttachment(attachmentDescriptions.size(), false);
		for (const auto& subpass : m_Specification.Pass->GetSpecification().Subpasses)
		{
			for (const auto& inputAttachment : subpass.InputAttachments)
			{
				isInputAttachment[inputAttachment] = true;
			}
		}

		VulkanAllocator allocator = VulkanAllocator(device, "Framebuffer");

		std::vector<vk::ImageView> attachments;
		for (uint32 i = 0; i < attachmentDescriptions.size(); i++)
		{
			const auto& attachment = attachmentDescriptions[i];
			TextureSpecification textureSpec;
			textureSpec.Width = width;
			textureSpec.Height = height;
			textureSpec.UseMipmap = false;
			textureSpec.SampleCount = attachment.Samples;
			textureSpec.Update = false;
			textureSpec.Format = attachment.Format;
			textureSpec.UsageFlags = attachment.Format == TextureFormat::Depth ? TextureUsageFlagBits::DepthAttachment
																			   : TextureUsageFlagBits::ColorAttachment;

			if (isInputAttachment[i])
			{
				textureSpec.UsageFlags |= TextureUsageFlagBits::InputAttachment;
			}
			if (attachment.Sampled)
			{
				textureSpec.UsageFlags |= TextureUsageFlagBits::ShaderRead;
				m_SampledImageIndex = i;
			}

			SharedRef<VulkanTexture2D> vulkanTexture2D = SharedRef<VulkanTexture2D>::Create(textureSpec);
			m_Textures.emplace_back(vulkanTexture2D);
			attachments.push_back(vulkanTexture2D->GetView(0));
		}

		vk::FramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.renderPass = static_cast<VkRenderPass>(m_Specification.Pass->GetHandle());
		framebufferCreateInfo.attachmentCount = static_cast<uint32>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = width;
		framebufferCreateInfo.height = height;
		framebufferCreateInfo.layers = 1;

		m_Handle = device->GetHandle().createFramebufferUnique(framebufferCreateInfo);
	}

} // namespace Neon
