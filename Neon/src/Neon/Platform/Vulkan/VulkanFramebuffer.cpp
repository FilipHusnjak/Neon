#include "neopch.h"

#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "Neon/Platform/Vulkan/VulkanRenderPass.h"
#include "Neon/Platform/Vulkan/VulkanTexture.h"
#include "VulkanFrameBuffer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Neon
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
		: Framebuffer(spec)
	{
		NEO_CORE_ASSERT(m_Specification.Pass);

		const auto device = VulkanContext::GetDevice();

		vk::DescriptorPoolSize poolSize = {vk::DescriptorType::eCombinedImageSampler, 1};
		vk::DescriptorPoolCreateInfo descPoolCreateInfo = {};
		descPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		descPoolCreateInfo.maxSets = 1;
		descPoolCreateInfo.poolSizeCount = 1;
		descPoolCreateInfo.pPoolSizes = &poolSize;
		m_DescPool = device->GetHandle().createDescriptorPoolUnique(descPoolCreateInfo);

		vk::DescriptorSetLayoutBinding binding = {};
		binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		binding.descriptorCount = 1;
		binding.stageFlags = vk::ShaderStageFlagBits::eFragment;

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.pBindings = &binding;
		m_ColorImageDescSetLayout = device->GetHandle().createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);

		vk::DescriptorSetAllocateInfo descAllocInfo = {};
		descAllocInfo.descriptorPool = m_DescPool.get();
		descAllocInfo.descriptorSetCount = 1;
		descAllocInfo.pSetLayouts = &m_ColorImageDescSetLayout.get();
		m_ColorImageDescSet = std::move(device->GetHandle().allocateDescriptorSetsUnique(descAllocInfo)[0]);

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
			vk::ImageUsageFlags usage = attachment.Format == TextureFormat::Depth ? vk::ImageUsageFlagBits::eDepthStencilAttachment
																				  : vk::ImageUsageFlagBits::eColorAttachment;

			if (isInputAttachment[i])
			{
				usage |= vk::ImageUsageFlagBits::eInputAttachment;
			}
			if (attachment.Sampled)
			{
				usage |= vk::ImageUsageFlagBits::eSampled;
				m_SampledImageIndex = i;
			}

			SharedRef<VulkanTexture2D> vulkanTexture2D =
				SharedRef<VulkanTexture2D>::Create(attachment.Format, width, height, attachment.Samples, usage);
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

		if (m_SampledImageIndex > -1)
		{
			vk::DescriptorImageInfo imageInfo = m_Textures[m_SampledImageIndex].As<VulkanTexture2D>()->GetTextureDescription(0);
			vk::WriteDescriptorSet descWrite = {};
			descWrite.dstSet = m_ColorImageDescSet.get();
			descWrite.descriptorCount = 1;
			descWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			descWrite.pImageInfo = &imageInfo;

			device->GetHandle().updateDescriptorSets({descWrite}, nullptr);
		}
	}

} // namespace Neon
