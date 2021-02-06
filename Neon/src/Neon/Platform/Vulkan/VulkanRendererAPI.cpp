#include "neopch.h"

#include "Renderer/Mesh.h"
#include "Renderer/PerspectiveCameraController.h"
#include "VulkanContext.h"
#include "VulkanFramebuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanRendererAPI.h"
#include "VulkanVertexBuffer.h"

namespace Neon
{
	VulkanRendererAPI::~VulkanRendererAPI()
	{
	}

	void VulkanRendererAPI::Init()
	{
		RendererAPI::Init();

		vk::PhysicalDeviceProperties props = VulkanContext::GetDevice()->GetPhysicalDevice()->GetProperties();

		RendererAPI::RenderAPICapabilities& caps = RendererAPI::GetCapabilities();
		caps.Vendor = props.deviceName.operator std::string();
		caps.Renderer = "Vulkan";
		caps.Version = "1.2";
	}

	void VulkanRendererAPI::Begin()
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();

		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();
		vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
		renderCommandBuffer.begin(beginInfo);
	}

	void VulkanRendererAPI::BeginRenderPass(const SharedRef<RenderPass>& renderPass)
	{
		renderPass->Begin();
	}

	void VulkanRendererAPI::SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform)
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();

		SharedRef<VulkanShader> meshShader = mesh->GetShader().As<VulkanShader>();

		SharedRef<VulkanGraphicsPipeline> meshPipeline = mesh->GetGraphicsPipeline().As<VulkanGraphicsPipeline>();

		renderCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, (VkPipeline)meshPipeline->GetHandle());

		renderCommandBuffer.bindVertexBuffers(0, {(VkBuffer)mesh->GetVertexBuffer()->GetHandle()}, {0});
		renderCommandBuffer.bindIndexBuffer((VkBuffer)mesh->GetIndexBuffer()->GetHandle(), 0, vk::IndexType::eUint32);

		renderCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshPipeline->GetLayout(), 0, 1,
											   &meshShader->GetDescriptorSet(), 0, nullptr);
		const auto& submeshes = mesh->GetSubmeshes();
		for (const auto& submesh : submeshes)
		{
			renderCommandBuffer.drawIndexed(submesh.IndexCount, 1, submesh.BaseIndex, submesh.BaseVertex, 0);
		}
	}

	void VulkanRendererAPI::SubmitFullscreenQuad(const SharedRef<GraphicsPipeline>& graphicsPipeline)
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();

		const SharedRef<VulkanGraphicsPipeline> vulkanPipeline = graphicsPipeline.As<VulkanGraphicsPipeline>();
		const SharedRef<VulkanShader> shader = graphicsPipeline->GetSpecification().Shader.As<VulkanShader>();
		renderCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vulkanPipeline->GetLayout(), 0, 1,
											   &shader->GetDescriptorSet(), 0, nullptr);
		renderCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, (VkPipeline)graphicsPipeline->GetHandle());

		renderCommandBuffer.bindVertexBuffers(0, {(VkBuffer)m_QuadVertexBuffer->GetHandle()}, {0});
		renderCommandBuffer.bindIndexBuffer((VkBuffer)m_QuadIndexBuffer->GetHandle(), 0, vk::IndexType::eUint32);

		renderCommandBuffer.drawIndexed(6, 1, 0, 0, 0);
	}

	void VulkanRendererAPI::DispatchCompute(const SharedRef<ComputePipeline>& computePipeline, uint32 groupCountX,
											uint32 groupCountY, uint32 groupCountZ)
	{
		const auto& device = VulkanContext::GetDevice();
		vk::CommandBuffer computeCommandBuffer = device->GetComputeCommandBuffer(true);

		const SharedRef<VulkanComputePipeline> vulkanPipeline = computePipeline.As<VulkanComputePipeline>();
		const SharedRef<VulkanShader> shader = computePipeline->GetSpecification().Shader.As<VulkanShader>();
		computeCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, vulkanPipeline->GetLayout(), 0, 1,
												&shader->GetDescriptorSet(), 0, nullptr);
		computeCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, (VkPipeline)computePipeline->GetHandle());
		computeCommandBuffer.dispatch(groupCountX, groupCountY, groupCountZ);

		device->FlushComputeCommandBuffer(computeCommandBuffer);

		Renderer::WaitIdle();
	}

	void VulkanRendererAPI::EndRenderPass()
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();
		renderCommandBuffer.endRenderPass();
	}

	void VulkanRendererAPI::End()
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();
		renderCommandBuffer.end();
	}

	void VulkanRendererAPI::WaitIdle()
	{
		VulkanContext::GetDevice()->GetHandle().waitIdle();
	}

	void VulkanRendererAPI::Shutdown()
	{
		RendererAPI::Shutdown();

		VulkanContext::GetDevice()->GetHandle().waitIdle();
	}

} // namespace Neon
