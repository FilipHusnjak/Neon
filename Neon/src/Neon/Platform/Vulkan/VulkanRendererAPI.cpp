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
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();

		uint32 width = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().Width;
		uint32 height = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().Height;

		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();

		std::vector<vk::ClearValue> clearValues;
		if (renderPass->GetSpecification().HasColor)
		{
			vk::ClearValue& clearValue = clearValues.emplace_back();
			glm::vec4 clearColor = renderPass->GetSpecification().TargetFramebuffer->GetSpecification().ClearColor;
			clearValue.color = *(vk::ClearColorValue*)(&clearColor);
		}
		if (renderPass->GetSpecification().HasDepth)
		{
			vk::ClearValue& clearValue = clearValues.emplace_back();
			clearValue.depthStencil = {1.0f, 0};
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
		renderPassBeginInfo.framebuffer = (VkFramebuffer)renderPass->GetSpecification().TargetFramebuffer->GetHandle();

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

	void VulkanRendererAPI::SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform)
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();

		SharedRef<VulkanShader> meshShader = mesh->GetShader().As<VulkanShader>();

		SharedRef<VulkanPipeline> meshPipeline = mesh->GetPipeline().As<VulkanPipeline>();

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

	void VulkanRendererAPI::SubmitFullscreenQuad(const SharedRef<Pipeline>& pipeline)
	{
		const VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapChain();
		vk::CommandBuffer renderCommandBuffer = swapChain.GetCurrentRenderCommandBuffer();

		const SharedRef<VulkanPipeline> vulkanPipeline = pipeline.As<VulkanPipeline>();
		const SharedRef<VulkanShader> shader = pipeline->GetSpecification().Shader.As<VulkanShader>();
		renderCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vulkanPipeline->GetLayout(), 0, 1,
											   &shader->GetDescriptorSet(), 0, nullptr);
		renderCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, (VkPipeline)pipeline->GetHandle());

		renderCommandBuffer.bindVertexBuffers(0, {(VkBuffer)m_QuadVertexBuffer->GetHandle()}, {0});
		renderCommandBuffer.bindIndexBuffer((VkBuffer)m_QuadIndexBuffer->GetHandle(), 0, vk::IndexType::eUint32);

		renderCommandBuffer.drawIndexed(6, 1, 0, 0, 0);
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
