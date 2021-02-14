#include "neopch.h"

#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "Neon/Platform/Vulkan/VulkanPipeline.h"
#include "Neon/Platform/Vulkan/VulkanShader.h"
#include "VulkanCommandBuffer.h"

namespace Neon
{
	VulkanCommandBuffer::VulkanCommandBuffer(const SharedRef<CommandPool>& commandPool)
		: CommandBuffer(commandPool)
	{
		vk::CommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.commandPool = (VkCommandPool)commandPool->GetHandle();
		cmdBufAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		cmdBufAllocateInfo.commandBufferCount = 1;

		m_Handle = std::move(VulkanContext::GetDevice()->GetHandle().allocateCommandBuffersUnique(cmdBufAllocateInfo)[0]);
	}

	void VulkanCommandBuffer::Begin() const
	{
		vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
		m_Handle.get().begin(beginInfo);
	}

	void VulkanCommandBuffer::End() const
	{
		m_Handle.get().end();
	}

	void VulkanCommandBuffer::Submit()
	{
		const auto& device = VulkanContext::GetDevice();
		vk::Queue queue;
		switch (m_Pool->GetType())
		{
			case CommandBufferType::Graphics:
				queue = device->GetGraphicsQueue();
				break;
			case CommandBufferType::Compute:
				queue = device->GetComputeQueue();
				break;
			case CommandBufferType::Transfer:
				queue = device->GetTransferQueue();
				break;
			default:
				NEO_CORE_ERROR("Unknown command buffer type!");
				break;
		}

		vk::SubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_Handle.get();
		submitInfo.pWaitDstStageMask = &m_WaitStage;
		submitInfo.pWaitSemaphores = m_WaitSemaphores.data();
		submitInfo.waitSemaphoreCount = static_cast<uint32>(m_WaitSemaphores.size());
		submitInfo.pSignalSemaphores = m_SignalSemaphores.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32>(m_SignalSemaphores.size());

		queue.submit(submitInfo, m_Fence);

		m_WaitSemaphores.clear();
		m_SignalSemaphores.clear();
		m_WaitStage = {};
		m_Fence = vk::Fence();
	}

	void VulkanCommandBuffer::BeginRenderPass(const SharedRef<RenderPass>& renderPass) const
	{
		uint32 width = renderPass->GetTargetFramebuffer()->GetSpecification().Width;
		uint32 height = renderPass->GetTargetFramebuffer()->GetSpecification().Height;

		std::vector<vk::ClearValue> clearValues;
		for (const auto& attachment : renderPass->GetSpecification().Attachments)
		{
			vk::ClearValue& clearValue = clearValues.emplace_back();
			if (attachment.Format == TextureFormat::Depth)
			{
				clearValue.depthStencil = {1.0f, 0};
			}
			else
			{
				clearValue.color = *(vk::ClearColorValue*)(&renderPass->GetSpecification().ClearColor);
			}
		}

		NEO_CORE_ASSERT(!clearValues.empty());

		vk::RenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.renderPass = (VkRenderPass)renderPass->GetHandle();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.framebuffer = (VkFramebuffer)renderPass->GetTargetFramebuffer()->GetHandle();

		// Update viewport state
		vk::Viewport sceneViewport = {};
		sceneViewport.x = 0.f;
		sceneViewport.y = 0.f;
		sceneViewport.width = (float)width;
		sceneViewport.height = (float)height;
		sceneViewport.minDepth = 0.f;
		sceneViewport.maxDepth = 1.f;

		m_Handle.get().setViewport(0, 1, &sceneViewport);

		// Update scissor state
		vk::Rect2D sceneCcissor = {};
		sceneCcissor.offset.x = 0;
		sceneCcissor.offset.y = 0;
		sceneCcissor.extent.width = width;
		sceneCcissor.extent.height = height;

		m_Handle.get().setScissor(0, 1, &sceneCcissor);

		m_Handle.get().beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	}

	void VulkanCommandBuffer::EndRenderPass() const
	{
		m_Handle.get().endRenderPass();
	}

	void VulkanCommandBuffer::BindVertexBuffer(const SharedRef<VertexBuffer>& vertexBuffer) const
	{
		m_Handle.get().bindVertexBuffers(0, {(VkBuffer)vertexBuffer->GetHandle()}, {0});
	}

	void VulkanCommandBuffer::BindIndexBuffer(const SharedRef<IndexBuffer>& indexBuffer) const
	{
		m_Handle.get().bindIndexBuffer((VkBuffer)indexBuffer->GetHandle(), 0, vk::IndexType::eUint32);
	}

	void VulkanCommandBuffer::BindPipeline(const SharedRef<Pipeline>& pipeline) const
	{
		SharedRef<VulkanShader> vulkanShader = pipeline->GetShader().As<VulkanShader>();

		m_Handle.get().bindPipeline(NeonToVulkanPipelineBindPoint(pipeline->GetBindPoint()), (VkPipeline)pipeline->GetHandle());
		m_Handle.get().bindDescriptorSets(NeonToVulkanPipelineBindPoint(pipeline->GetBindPoint()),
										  (VkPipelineLayout)pipeline->GetLayout(), 0, 1, &vulkanShader->m_DescriptorSet.get(), 0,
										  nullptr);

		for (const auto& [name, pushConstant] : vulkanShader->m_PushConstants)
		{
			m_Handle.get().pushConstants((VkPipelineLayout)pipeline->GetLayout(), pushConstant.ShaderStage, 0, pushConstant.Size,
										 pushConstant.Data.get());
		}
	}

	void VulkanCommandBuffer::DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset,
										  uint32 firstInstance) const
	{
		m_Handle.get().drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::Dispatch(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ) const
	{
		m_Handle.get().dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void VulkanCommandBuffer::AddSignalSemaphore(vk::Semaphore signalSemaphore)
	{
		m_SignalSemaphores.push_back(signalSemaphore);
	}

	void VulkanCommandBuffer::AddWaitSemaphore(vk::Semaphore waitSemaphore)
	{
		m_WaitSemaphores.push_back(waitSemaphore);
	}

	void VulkanCommandBuffer::SetFence(vk::Fence fence)
	{
		m_Fence = fence;
	}

	void VulkanCommandBuffer::SetWaitStage(vk::PipelineStageFlags waitStage)
	{
		m_WaitStage = waitStage;
	}

	VulkanCommandPool::VulkanCommandPool(CommandBufferType type)
		: CommandPool(type)
	{
		vk::CommandPoolCreateInfo cmdPoolInfo = {};
		switch (type)
		{
			case CommandBufferType::Graphics:
				cmdPoolInfo.queueFamilyIndex = VulkanContext::GetDevice()->GetPhysicalDevice()->GetGraphicsQueueIndex();
				break;
			case CommandBufferType::Compute:
				cmdPoolInfo.queueFamilyIndex = VulkanContext::GetDevice()->GetPhysicalDevice()->GetComputeQueueIndex();
				break;
			case CommandBufferType::Transfer:
				cmdPoolInfo.queueFamilyIndex = VulkanContext::GetDevice()->GetPhysicalDevice()->GetTransferQueueIndex();
				break;
			default:
				NEO_CORE_ERROR("Unknown command buffer type!");
				break;
		}

		cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		m_Handle = VulkanContext::GetDevice()->GetHandle().createCommandPoolUnique(cmdPoolInfo);
	}

} // namespace Neon
