#pragma once

#include "Neon/Platform/Vulkan/Vulkan.h"
#include "Neon/Platform/Vulkan/VulkanDevice.h"
#include "Neon/Platform/Vulkan/VulkanSwapChain.h"
#include "Neon/Renderer/RendererContext.h"

struct GLFWwindow;

namespace Neon
{
	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext() = default;

		void Init() override;

		void BeginFrame() override;
		void SwapBuffers() override;

		void OnResize(uint32 width, uint32 height) override;

		uint32 GetTargetMaxFramesInFlight() const override
		{
			return m_SwapChain.GetTargetMaxFramesInFlight();
		}

		static vk::Instance GetInstance()
		{
			return s_Instance.get();
		}

		static SharedRef<VulkanContext> Get()
		{
			return SharedRef<VulkanContext>(RendererContext::Get());
		}

		static SharedRef<VulkanDevice> GetDevice()
		{
			return Get()->m_Device;
		}

		const SharedRef<CommandBuffer>& GetPrimaryRenderCommandBuffer() const override
		{
			return m_RenderCommandBuffers[m_SwapChain.GetCurrentFrameIndex()];
		}

		const VulkanSwapChain& GetSwapChain() const
		{
			return m_SwapChain;
		}

		void WaitIdle() const override
		{
			m_Device->GetHandle().waitIdle();
		}

		SharedRef<CommandBuffer> GetCommandBuffer(CommandBufferType type, bool begin) const override;
		void SubmitCommandBuffer(SharedRef<CommandBuffer>& commandBuffer) const override;

	private:
		GLFWwindow* m_WindowHandle;

		inline static vk::UniqueInstance s_Instance;

		vk::UniqueDebugReportCallbackEXT m_DebugReportCallback;

		SharedRef<VulkanDevice> m_Device;

		SharedRef<VulkanPhysicalDevice> m_PhysicalDevice;

		VulkanSwapChain m_SwapChain;

		const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
		std::vector<const char*> m_InstanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME,
														 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
														 VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME};

		SharedRef<CommandPool> m_GraphicsCommandPool;
		SharedRef<CommandPool> m_ComputeCommandPool;

		std::vector<SharedRef<CommandBuffer>> m_RenderCommandBuffers;

		friend class VulkanSwapChain;
	};

} // namespace Neon
