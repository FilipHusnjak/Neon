#include "neopch.h"

#include "VulkanContext.h"
#include "VulkanRenderPass.h"

namespace Neon
{
	VulkanRenderPass::VulkanRenderPass(RenderPassSpecification specification)
		: RenderPass(specification)
	{
		const auto& device = VulkanContext::GetDevice();

		vk::SubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

		// TODO: Multisampling

		std::vector<vk::AttachmentDescription> attachmentDescriptions;

		vk::AttachmentReference colorReference;
		// COLOR ATTACHMENT
		if (m_Specification.HasColor)
		{
			const vk::Format COLOR_BUFFER_FORMAT = FramebufferFormatToVulkanFormat(m_Specification.ColorFormat);

			vk::AttachmentDescription colorDesc;
			colorDesc.format = COLOR_BUFFER_FORMAT;
			colorDesc.samples = vk::SampleCountFlagBits::e1;
			colorDesc.loadOp = vk::AttachmentLoadOp::eClear;
			colorDesc.storeOp = vk::AttachmentStoreOp::eStore;
			colorDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			colorDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			colorDesc.initialLayout = vk::ImageLayout::eUndefined;
			colorDesc.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			attachmentDescriptions.push_back(colorDesc);

			colorReference = {static_cast<uint32>(attachmentDescriptions.size()) - 1, vk::ImageLayout::eColorAttachmentOptimal};
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorReference;
		}

		vk::AttachmentReference depthReference;
		// DEPTH ATTACHMENT
		if (m_Specification.HasDepth)
		{
			const vk::Format depthFormat = device->GetPhysicalDevice()->GetDepthFormat();

			vk::AttachmentDescription depthDesc;
			depthDesc.format = depthFormat;
			depthDesc.samples = vk::SampleCountFlagBits::e1;
			depthDesc.loadOp = vk::AttachmentLoadOp::eClear;
			depthDesc.storeOp = vk::AttachmentStoreOp::eDontCare;
			depthDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			depthDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			depthDesc.initialLayout = vk::ImageLayout::eUndefined;
			depthDesc.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			attachmentDescriptions.push_back(depthDesc);

			depthReference = {static_cast<uint32>(attachmentDescriptions.size()) - 1, vk::ImageLayout::eDepthStencilAttachmentOptimal};
			subpassDescription.pDepthStencilAttachment = &depthReference;
		}

		// Use subpass dependencies for layout transitions
		std::array<vk::SubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
		dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;
		dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

		// Create renderpass
		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		m_Handle = device->GetHandle().createRenderPassUnique(renderPassInfo);
	}

} // namespace Neon
