#include "neopch.h"

#include "VulkanContext.h"
#include "VulkanRenderPass.h"

namespace Neon
{
	VulkanRenderPass::VulkanRenderPass(RenderPassSpecification specification)
		: RenderPass(specification)
	{
		const auto& device = VulkanContext::GetDevice();

		std::vector<std::vector<vk::AttachmentReference>> inputAttachmentReferences{specification.Subpasses.size()};
		std::vector<std::vector<vk::AttachmentReference>> colorAttachmentReferences{specification.Subpasses.size()};
		std::vector<std::vector<vk::AttachmentReference>> depthStencilAttachmentReferences{specification.Subpasses.size()};
		std::vector<std::vector<vk::AttachmentReference>> colorResolveAttachmentReferences{specification.Subpasses.size()};
		std::vector<std::vector<vk::AttachmentReference>> depthResolveAttachmentReferences{specification.Subpasses.size()};

		std::vector<vk::SubpassDescription> subpassDescriptions;
		for (uint32 i = 0; i < specification.Subpasses.size(); i++)
		{
			auto& subpass = specification.Subpasses[i];

			// Fill color attachment references
			for (auto colorAttachment : subpass.ColorAttachments)
			{
				if (specification.Attachments[colorAttachment].Format != AttachmentFormat::Depth)
				{
					colorAttachmentReferences[i].emplace_back(colorAttachment, vk::ImageLayout::eColorAttachmentOptimal);
				}
			}

			// Fill input attachment references
			for (auto inputAttachment : subpass.InputAttachments)
			{
				vk::ImageLayout layout = specification.Attachments[inputAttachment].Format == AttachmentFormat::Depth
											 ? vk::ImageLayout::eDepthStencilReadOnlyOptimal
											 : vk::ImageLayout::eShaderReadOnlyOptimal;
				inputAttachmentReferences[i].emplace_back(inputAttachment, layout);
			}

			// Fill color resolve attachment references
			for (auto colorResolveAttachment : subpass.ColorResolveAttachments)
			{
				colorResolveAttachmentReferences[i].emplace_back(colorResolveAttachment, vk::ImageLayout::eColorAttachmentOptimal);
			}

			if (subpass.EnableDepthStencil)
			{
				for (uint32 j = 0; j < specification.Attachments.size(); j++)
				{
					if (specification.Attachments[j].Format == AttachmentFormat::Depth)
					{
						depthStencilAttachmentReferences[i].emplace_back(j, vk::ImageLayout::eDepthStencilAttachmentOptimal);
						break;
					}
				}
			}

			auto& subpassDescription = subpassDescriptions.emplace_back();
			subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

			subpassDescription.inputAttachmentCount = static_cast<uint32>(inputAttachmentReferences[i].size());
			subpassDescription.pInputAttachments = inputAttachmentReferences[i].data();

			subpassDescription.colorAttachmentCount = static_cast<uint32>(colorAttachmentReferences[i].size());
			subpassDescription.pColorAttachments = colorAttachmentReferences[i].data();

			subpassDescription.pResolveAttachments = colorResolveAttachmentReferences[i].data();

			subpassDescription.pDepthStencilAttachment = depthStencilAttachmentReferences[i].data();
		}

		if (subpassDescriptions.empty())
		{
			colorAttachmentReferences.emplace_back();

			vk::SubpassDescription subpassDescription;
			subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

			for (uint32 i = 0; i < specification.Attachments.size(); i++)
			{
				if (specification.Attachments[i].Format != AttachmentFormat::Depth)
				{
					colorAttachmentReferences[0].emplace_back(i, vk::ImageLayout::eColorAttachmentOptimal);
				}
			}

			NEO_CORE_ASSERT(!(colorAttachmentReferences[0]).empty());

			subpassDescription.colorAttachmentCount = static_cast<uint32>(colorAttachmentReferences[0].size());
			subpassDescription.pColorAttachments = colorAttachmentReferences[0].data();

			subpassDescriptions.push_back(subpassDescription);
		}

		std::vector<vk::SubpassDependency> subpassDependencies(subpassDescriptions.size());
		for (uint32 i = 0; i < subpassDescriptions.size() - 1; i++)
		{
			// Transition input attachments from color attachment to shader read
			subpassDependencies[i].srcSubpass = i;
			subpassDependencies[i].dstSubpass = i + 1;
			subpassDependencies[i].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			subpassDependencies[i].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
			subpassDependencies[i].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			subpassDependencies[i].dstAccessMask = vk::AccessFlagBits::eInputAttachmentRead;
			subpassDependencies[i].dependencyFlags = vk::DependencyFlagBits::eByRegion;
		}

		// TODO: Check if this is correct?
		// Dependency needed when reading from attachments written to by this renderpass inside next renderpass
		subpassDependencies[0].srcSubpass = 0;
		subpassDependencies[0].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[0].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		subpassDependencies[0].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		subpassDependencies[0].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		subpassDependencies[0].dstAccessMask = vk::AccessFlagBits::eShaderRead;
		subpassDependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

		std::vector<vk::AttachmentDescription> attachmentDescriptions;
		for (const auto& attachment : specification.Attachments)
		{
			auto& attachmentDescription = attachmentDescriptions.emplace_back();
			attachmentDescription.format = attachment.Format == AttachmentFormat::Depth
											   ? device->GetPhysicalDevice()->GetDepthFormat()
											   : ConvertAttachmentFormatToVulkan(attachment.Format);
			attachmentDescription.loadOp = ConvertLoadOpToVulkan(attachment.LoadOp);
			attachmentDescription.storeOp = ConvertStoreOpToVulkan(attachment.StoreOp);
			attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		}

		// TODO
		// Make the initial layout same as in the first subpass using that attachment
		/*for (auto& subpass : subpassDescriptions)
		{
			for (uint32 i = 0; i < subpass.colorAttachmentCount; i++)
			{
				auto& reference = subpass.pColorAttachments[i];
				// Set it only if not defined yet
				if (attachmentDescriptions[reference.attachment].initialLayout == vk::ImageLayout::eUndefined)
				{
					attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
				}
			}

			for (uint32 i = 0; i < subpass.inputAttachmentCount; i++)
			{
				auto& reference = subpass.pInputAttachments[i];
				// Set it only if not defined yet
				if (attachmentDescriptions[reference.attachment].initialLayout == vk::ImageLayout::eUndefined)
				{
					attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
				}
			}

			if (subpass.pDepthStencilAttachment)
			{
				auto& reference = *subpass.pDepthStencilAttachment;
				// Set it only if not defined yet
				if (attachmentDescriptions[reference.attachment].initialLayout == vk::ImageLayout::eUndefined)
				{
					attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
				}
			}

			if (subpass.pResolveAttachments)
			{
				for (uint32 i = 0; i < subpass.colorAttachmentCount; i++)
				{
					auto& reference = subpass.pResolveAttachments[i];
					// Set it only if not defined yet
					if (attachmentDescriptions[reference.attachment].initialLayout == vk::ImageLayout::eUndefined)
					{
						attachmentDescriptions[reference.attachment].initialLayout = reference.layout;
					}
				}
			}
		}*/

		// Make the final layout same as the last subpass layout
		{
			auto& subpass = subpassDescriptions.back();

			for (uint32 i = 0; i < subpass.colorAttachmentCount; i++)
			{
				const auto& reference = subpass.pColorAttachments[i];

				attachmentDescriptions[reference.attachment].finalLayout = reference.layout;
			}

			for (uint32 i = 0; i < subpass.inputAttachmentCount; i++)
			{
				const auto& reference = subpass.pInputAttachments[i];

				attachmentDescriptions[reference.attachment].finalLayout = reference.layout;

				// Do not use depth attachment if used as input
				if (specification.Attachments[reference.attachment].Format == AttachmentFormat::Depth)
				{
					subpass.pDepthStencilAttachment = nullptr;
				}
			}

			if (subpass.pDepthStencilAttachment)
			{
				const auto& reference = *subpass.pDepthStencilAttachment;

				attachmentDescriptions[reference.attachment].finalLayout = reference.layout;
			}

			if (subpass.pResolveAttachments)
			{
				for (uint32 i = 0; i < subpass.colorAttachmentCount; i++)
				{
					const auto& reference = subpass.pResolveAttachments[i];

					attachmentDescriptions[reference.attachment].finalLayout = reference.layout;
				}
			}
		}

		// If attachment is sampled later, transition it to shader read layout
		for (uint32 i = 0; i < specification.Attachments.size(); i++)
		{
			if (specification.Attachments[i].Sampled)
			{
				attachmentDescriptions[i].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			}
		}

		// Create renderpass
		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = static_cast<uint32>(subpassDescriptions.size());
		renderPassInfo.pSubpasses = subpassDescriptions.data();
		renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
		renderPassInfo.pDependencies = subpassDependencies.data();

		m_Handle = device->GetHandle().createRenderPassUnique(renderPassInfo);
	}

	void VulkanRenderPass::Begin() const
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();

		uint32 width = m_TargetFramebuffer->GetSpecification().Width;
		uint32 height = m_TargetFramebuffer->GetSpecification().Height;

		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();

		std::vector<vk::ClearValue> clearValues;
		for (const auto& attachment : m_Specification.Attachments)
		{
			vk::ClearValue& clearValue = clearValues.emplace_back();
			if (attachment.Format == AttachmentFormat::Depth)
			{
				clearValue.depthStencil = {1.0f, 0};
			}
			else
			{
				clearValue.color = *(vk::ClearColorValue*)(&m_Specification.ClearColor);
			}
		}

		NEO_CORE_ASSERT(!clearValues.empty());

		vk::RenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.renderPass = m_Handle.get();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.framebuffer = (VkFramebuffer)m_TargetFramebuffer->GetHandle();

		// Update viewport state
		vk::Viewport sceneViewport = {};
		sceneViewport.x = 0.f;
		sceneViewport.y = 0.f;
		sceneViewport.width = (float)width;
		sceneViewport.height = (float)height;
		sceneViewport.minDepth = 0.f;
		sceneViewport.maxDepth = 1.f;

		renderCommandBuffer.setViewport(0, 1, &sceneViewport);

		// Update scissor state
		vk::Rect2D sceneCcissor = {};
		sceneCcissor.offset.x = 0;
		sceneCcissor.offset.y = 0;
		sceneCcissor.extent.width = width;
		sceneCcissor.extent.height = height;

		renderCommandBuffer.setScissor(0, 1, &sceneCcissor);

		renderCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	}

} // namespace Neon
