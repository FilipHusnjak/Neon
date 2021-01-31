#include "neopch.h"

#include "VulkanContext.h"
#include "VulkanFrameBuffer.h"
#include "VulkanRenderPass.h"

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

		// Create sampler to sample from the attachment in the fragment shader
		vk::SamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.minFilter = vk::Filter::eLinear;
		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 1.0f;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		m_AttachmentSampler = device->GetHandle().createSamplerUnique(samplerCreateInfo);

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

	vk::ImageView VulkanFramebuffer::GetSampledImageView(uint32 index)
	{
		NEO_CORE_ASSERT(m_Specification.Pass->GetSpecification().Attachments[index].Sampled,
						"Attachment at given index is not specified as sampled!");
		return m_Attachments[index].View.get();
	}

	void VulkanFramebuffer::Create(uint32 width, uint32 height)
	{
		const auto& device = VulkanContext::GetDevice();

		m_Attachments.clear();

		// Fill a descriptor for later use in a descriptor set
		m_AttachmentDescriptorInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_AttachmentDescriptorInfo.sampler = m_AttachmentSampler.get();

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
			vk::Format format = attachment.Format == AttachmentFormat::Depth ? device->GetPhysicalDevice()->GetDepthFormat()
																			 : ConvertAttachmentFormatToVulkan(attachment.Format);

			vk::ImageCreateInfo imageCreateInfo = {};
			imageCreateInfo.imageType = vk::ImageType::e2D;
			imageCreateInfo.format = format;
			imageCreateInfo.extent.width = width;
			imageCreateInfo.extent.height = height;
			imageCreateInfo.extent.depth = 1;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = ConvertSampleCountToVulkan(attachment.Samples);
			imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
			imageCreateInfo.usage = attachment.Format == AttachmentFormat::Depth ? vk::ImageUsageFlagBits::eDepthStencilAttachment
																				 : vk::ImageUsageFlagBits::eColorAttachment;
			if (isInputAttachment[i])
			{
				imageCreateInfo.usage |= vk::ImageUsageFlagBits::eInputAttachment;
			}
			if (attachment.Sampled)
			{
				imageCreateInfo.usage |= vk::ImageUsageFlagBits::eSampled;
			}

			FrameBufferAttachment& framebufferAttachment = m_Attachments.emplace_back();
			framebufferAttachment.Image = device->GetHandle().createImageUnique(imageCreateInfo);

			vk::MemoryRequirements memReqs = device->GetHandle().getImageMemoryRequirements(framebufferAttachment.Image.get());
			allocator.Allocate(memReqs, framebufferAttachment.Memory);

			device->GetHandle().bindImageMemory(framebufferAttachment.Image.get(), framebufferAttachment.Memory.get(), 0);

			vk::ImageViewCreateInfo colorImageViewCreateInfo = {};
			colorImageViewCreateInfo.viewType = vk::ImageViewType::e2D;
			colorImageViewCreateInfo.format = format;
			colorImageViewCreateInfo.subresourceRange = {};
			colorImageViewCreateInfo.subresourceRange.aspectMask =
				attachment.Format == AttachmentFormat::Depth ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil
															 : vk::ImageAspectFlagBits::eColor;
			colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			colorImageViewCreateInfo.subresourceRange.levelCount = 1;
			colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			colorImageViewCreateInfo.subresourceRange.layerCount = 1;
			colorImageViewCreateInfo.image = framebufferAttachment.Image.get();
			framebufferAttachment.View = device->GetHandle().createImageViewUnique(colorImageViewCreateInfo);

			if (attachment.Sampled)
			{
				m_AttachmentDescriptorInfo.imageView = framebufferAttachment.View.get();
			}

			attachments.push_back(framebufferAttachment.View.get());
		}

		vk::FramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.renderPass = static_cast<VkRenderPass>(m_Specification.Pass->GetHandle());
		framebufferCreateInfo.attachmentCount = static_cast<uint32>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = width;
		framebufferCreateInfo.height = height;
		framebufferCreateInfo.layers = 1;

		m_Handle = device->GetHandle().createFramebufferUnique(framebufferCreateInfo);

		vk::WriteDescriptorSet descWrite = {};
		descWrite.dstSet = m_ColorImageDescSet.get();
		descWrite.descriptorCount = 1;
		descWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descWrite.pImageInfo = &m_AttachmentDescriptorInfo;

		device->GetHandle().updateDescriptorSets({descWrite}, nullptr);
	}

} // namespace Neon
