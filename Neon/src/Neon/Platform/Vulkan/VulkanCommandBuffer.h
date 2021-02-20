#pragma once

#include "Neon/Platform/Vulkan/Vulkan.h"
#include "Neon/Renderer/CommandBuffer.h"

namespace Neon
{
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(const SharedRef<CommandPool>& commandPool);

		void Begin() const override;
		void End() const override;
		void Submit() override;

		void BeginRenderPass(const SharedRef<RenderPass>& renderPass) const override;
		void EndRenderPass() const override;
		void BindVertexBuffer(const SharedRef<VertexBuffer>& vertexBuffer) const override;
		void BindIndexBuffer(const SharedRef<IndexBuffer>& indexBuffer) const override;
		void BindPipeline(const SharedRef<Pipeline>& pipeline) const override;
		void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset,
						 uint32 firstInstance) const override;
		void Dispatch(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ) const override;

		void AddSignalSemaphore(vk::Semaphore signalSemaphore);
		void AddWaitSemaphore(vk::Semaphore waitSemaphore);
		void SetFence(vk::Fence fence);
		void SetWaitStage(vk::PipelineStageFlags waitStage);

		void* GetHandle() const override
		{
			return m_Handle.get();
		}

	private:
		vk::UniqueCommandBuffer m_Handle;

		std::vector<vk::Semaphore> m_WaitSemaphores;
		std::vector<vk::Semaphore> m_SignalSemaphores;
		vk::PipelineStageFlags m_WaitStage;
		vk::Fence m_Fence;

		vk::UniqueDescriptorPool m_DescPool;
		vk::UniqueDescriptorSet m_DescSet;
	};

	class VulkanCommandPool : public CommandPool
	{
	public:
		VulkanCommandPool(CommandBufferType type);

		void* GetHandle() const override
		{
			return m_Handle.get();
		}

	private:
		vk::UniqueCommandPool m_Handle;
	};
} // namespace Neon
