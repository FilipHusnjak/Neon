#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/RendererContext.h"

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

struct GLFWwindow;

namespace Neon
{
	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext(GLFWwindow* windowHandle);
		virtual ~VulkanContext() = default;

		void BeginFrame() override;
		void SwapBuffers() override;

		void OnResize(uint32 width, uint32 height) override;

		const VulkanSwapChain& GetSwapChain() const
		{
			return m_SwapChain;
		}

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
			return SharedRef<VulkanContext>(Renderer::GetContext());
		}

		static SharedRef<VulkanDevice> GetDevice()
		{
			return Get()->m_Device;
		}

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

		friend class VulkanSwapChain;
	};

} // namespace Neon
