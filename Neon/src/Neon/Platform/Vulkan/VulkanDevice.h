#pragma once

#include "Core/Core.h"

#include "Vulkan.h"

namespace Neon
{
	class VulkanPhysicalDevice : public RefCounted
	{
	public:
		VulkanPhysicalDevice();
		~VulkanPhysicalDevice() = default;

		uint32 GetMemoryTypeIndex(uint32 typeBits, vk::MemoryPropertyFlags properties) const;

		vk::PhysicalDevice GetHandle() const
		{
			return m_Handle;
		}

		int32 GetGraphicsQueueIndex() const
		{
			return m_QueueFamilyIndices.Graphics;
		}

		vk::Format GetDepthFormat() const
		{
			return m_DepthFormat;
		}

		vk::PhysicalDeviceProperties GetProperties()
		{
			return m_Properties;
		}

		vk::PhysicalDeviceFeatures GetSupportedFeatures() const
		{
			return m_SupportedFeatures;
		}

		static SharedRef<VulkanPhysicalDevice> Select();

	private:
		struct QueueFamilyIndices
		{
			int32 Graphics = -1;
			int32 Compute = -1;
			int32 Transfer = -1;
		};
		QueueFamilyIndices GetQueueFamilyIndices(vk::QueueFlags queueFlags);
		vk::Format FindDepthFormat();

	private:
		QueueFamilyIndices m_QueueFamilyIndices;

		vk::PhysicalDevice m_Handle;
		vk::PhysicalDeviceProperties m_Properties;
		vk::PhysicalDeviceFeatures m_SupportedFeatures;
		vk::PhysicalDeviceMemoryProperties m_MemoryProperties;

		vk::Format m_DepthFormat = vk::Format::eUndefined;

		std::vector<vk::ExtensionProperties> m_SupportedExtensions;

		std::vector<vk::QueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<vk::DeviceQueueCreateInfo> m_QueueCreateInfos;

		const std::vector<const char*> m_RequiredPhysicalDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
																			 VK_KHR_MAINTENANCE2_EXTENSION_NAME,
																			 VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
																			 VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
																			 VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
																			 VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
																			 VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
																			 VK_KHR_MULTIVIEW_EXTENSION_NAME};

		friend class VulkanDevice;
	};

	class VulkanDevice : public RefCounted
	{
	public:
		VulkanDevice(SharedRef<VulkanPhysicalDevice>& physicalDevice);
		~VulkanDevice() = default;

		vk::Device GetHandle() const
		{
			return m_Handle.get();
		}

		SharedRef<VulkanPhysicalDevice> GetPhysicalDevice() const
		{
			return m_PhysicalDevice;
		}

		vk::Queue GetGraphicsQueue() const
		{
			return m_GraphicsQueue;
		}
		vk::Queue GetComputeQueue() const
		{
			return m_ComputeQueue;
		}
		vk::Queue GetTransferQueue() const
		{
			return m_TransferQueue;
		}

		vk::CommandPool GetGraphicsCommandPool() const
		{
			return m_GraphicsCommandPool.get();
		}

		vk::CommandBuffer GetGraphicsCommandBuffer(bool begin) const;
		void FlushGraphicsCommandBuffer(vk::CommandBuffer commandBuffer) const;

		vk::CommandBuffer GetComputeCommandBuffer(bool begin) const;
		void FlushComputeCommandBuffer(vk::CommandBuffer commandBuffer) const;

		vk::CommandBuffer CreateSecondaryCommandBuffer() const;

		static SharedRef<VulkanDevice> Create(SharedRef<VulkanPhysicalDevice>& physicalDevice);

	private:
		vk::UniqueDevice m_Handle;
		SharedRef<VulkanPhysicalDevice> m_PhysicalDevice;

		vk::Queue m_GraphicsQueue;
		vk::Queue m_ComputeQueue;
		vk::Queue m_TransferQueue;

		vk::UniqueCommandPool m_GraphicsCommandPool;
		vk::UniqueCommandPool m_ComputeCommandPool;

		const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
	};

} // namespace Neon
