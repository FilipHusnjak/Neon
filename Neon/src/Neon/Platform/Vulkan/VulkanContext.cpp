#include "neopch.h"

#include "Neon/Platform/Vulkan/VulkanCommandBuffer.h"
#include "Neon/Renderer/RendererAPI.h"
#include "VulkanContext.h"

#include <GLFW/glfw3.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace Neon
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(VkDebugReportFlagsEXT flags,
																	VkDebugReportObjectTypeEXT objectType, uint64_t object,
																	size_t location, int32_t messageCode, const char* pLayerPrefix,
																	const char* pMessage, void* pUserData)
	{
		NEO_CORE_WARN("VulkanDebugCallback:\n  Object Type: {0}\n  Message: {1}", objectType, pMessage);
		return VK_FALSE;
	}

	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		vk::DynamicLoader dl;
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		NEO_CORE_ASSERT(glfwVulkanSupported(), "Vulkan is not supported");

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		m_InstanceExtensions.insert(m_InstanceExtensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

		std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();
		for (const auto& extension : m_InstanceExtensions)
		{
			bool found = false;
			for (const auto& supportedExtension : supportedExtensions)
			{
				if (strcmp(supportedExtension.extensionName, extension) != 0)
				{
					found = true;
					break;
				}
			}
			NEO_CORE_ASSERT(found, "Not all required instance extensions are supported");
		}

		vk::ApplicationInfo applicationInfo("Neon", 1, "Vulkan engine", 1, VK_API_VERSION_1_2);
#ifdef NEO_DEBUG
		vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, static_cast<uint32>(m_ValidationLayers.size()),
												  m_ValidationLayers.data(), static_cast<uint32>(m_InstanceExtensions.size()),
												  m_InstanceExtensions.data());
#else
		vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, 0, nullptr,
												  static_cast<uint32_t>(m_InstanceExtensions.size()), m_InstanceExtensions.data());
#endif
		s_Instance = vk::createInstanceUnique(instanceCreateInfo);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(s_Instance.get());

		vk::DebugReportCallbackCreateInfoEXT debugReportCreateInfo = {};
		debugReportCreateInfo.flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning |
									  vk::DebugReportFlagBitsEXT::ePerformanceWarning;
		debugReportCreateInfo.pfnCallback = VulkanDebugReportCallback;

		m_DebugReportCallback = s_Instance.get().createDebugReportCallbackEXTUnique(debugReportCreateInfo, nullptr);

		m_PhysicalDevice = VulkanPhysicalDevice::Select();
		m_Device = VulkanDevice::Create(m_PhysicalDevice);
	}

	void VulkanContext::Init()
	{
		vk::PhysicalDeviceProperties props = m_PhysicalDevice->GetProperties();

		RendererAPI::RenderAPICapabilities& caps = RendererAPI::GetCapabilities();
		caps.Vendor = props.deviceName.operator std::string();
		caps.Renderer = "Vulkan";
		caps.Version = "1.2";

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Device->GetHandle());

		m_SwapChain.Init(s_Instance.get(), m_Device);
		m_SwapChain.InitSurface(m_WindowHandle);
		// This window size should be ignored
		uint32 width = 1920, height = 1080;
		m_SwapChain.Create(&width, &height);

		m_GraphicsCommandPool = CommandPool::Create(CommandBufferType::Graphics);
		m_ComputeCommandPool = CommandPool::Create(CommandBufferType::Compute);

		for (uint32 i = 0; i < m_SwapChain.GetTargetMaxFramesInFlight(); i++)
		{
			m_RenderCommandBuffers.push_back(CommandBuffer::Create(m_GraphicsCommandPool));
		}
	}

	void VulkanContext::BeginFrame()
	{
		m_SwapChain.BeginFrame();
		GetPrimaryRenderCommandBuffer()->Begin();
	}

	void VulkanContext::SwapBuffers()
	{
		GetPrimaryRenderCommandBuffer()->End();
		m_SwapChain.Present();
	}

	void VulkanContext::OnResize(uint32 width, uint32 height)
	{
		m_SwapChain.OnResize(width, height);
	}

	SharedRef<CommandBuffer> VulkanContext::GetCommandBuffer(CommandBufferType type, bool begin) const
	{
		SharedRef<CommandBuffer> commandBuffer;
		switch (type)
		{
			case CommandBufferType::Graphics:
				commandBuffer = CommandBuffer::Create(m_GraphicsCommandPool);
				break;
			case CommandBufferType::Compute:
				commandBuffer = CommandBuffer::Create(m_ComputeCommandPool);
				break;
			default:
				NEO_CORE_ASSERT(false, "Uknown command buffer type");
				break;
		}

		if (begin)
		{
			commandBuffer->Begin();
		}

		return commandBuffer;
	}

	void VulkanContext::SubmitCommandBuffer(SharedRef<CommandBuffer>& commandBuffer) const
	{
		NEO_CORE_ASSERT(commandBuffer);

		const uint64 DEFAULT_FENCE_TIMEOUT = 100000000000;

		const auto& device = VulkanContext::GetDevice();

		auto vulkanCommandBuffer = commandBuffer.As<VulkanCommandBuffer>();

		vulkanCommandBuffer->End();

		// Create fence to ensure that the command buffer has finished executing
		vk::FenceCreateInfo fenceCreateInfo = {};
		vk::UniqueFence fence = device->GetHandle().createFenceUnique(fenceCreateInfo);

		vulkanCommandBuffer->SetFence(fence.get());
		vulkanCommandBuffer->Submit();

		// Wait for the fence to signal that command buffer has finished executing
		device->GetHandle().waitForFences(fence.get(), VK_TRUE, DEFAULT_FENCE_TIMEOUT);
	}

} // namespace Neon
