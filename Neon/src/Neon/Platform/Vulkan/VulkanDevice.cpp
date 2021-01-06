#include "neopch.h"

#include "VulkanContext.h"
#include "VulkanDevice.h"

namespace Neon
{
	SharedRef<VulkanPhysicalDevice> VulkanPhysicalDevice::Select()
	{
		return SharedRef<VulkanPhysicalDevice>::Create();
	}

	VulkanPhysicalDevice::VulkanPhysicalDevice()
	{
		NEO_CORE_ASSERT(VulkanContext::GetInstance(), "Instance is not created");
		std::vector<vk::PhysicalDevice> devices = VulkanContext::GetInstance().enumeratePhysicalDevices();
		m_Handle = devices.front();
		for (auto& device : devices)
		{
			vk::PhysicalDeviceProperties properties = device.getProperties();
			if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				m_Handle = device;
				break;
			}
		}
		NEO_CORE_ASSERT(m_Handle, "Physical device was not found");

		m_Properties = m_Handle.getProperties();
		m_Features = m_Handle.getFeatures();
		m_MemoryProperties = m_Handle.getMemoryProperties();

		m_SupportedExtensions = m_Handle.enumerateDeviceExtensionProperties();

		m_QueueFamilyProperties = m_Handle.getQueueFamilyProperties();

		vk::QueueFlags requestedQueueTypes = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;
		m_QueueFamilyIndices = GetQueueFamilyIndices(requestedQueueTypes);

		static const float queuePriority = 1.f;
		if (requestedQueueTypes & vk::QueueFlagBits::eGraphics)
		{
			m_QueueCreateInfos.push_back({{}, static_cast<uint32>(m_QueueFamilyIndices.Graphics), 1, &queuePriority});
		}
		if (requestedQueueTypes & vk::QueueFlagBits::eCompute)
		{
			if (m_QueueFamilyIndices.Compute != m_QueueFamilyIndices.Graphics)
			{
				m_QueueCreateInfos.push_back({{}, static_cast<uint32>(m_QueueFamilyIndices.Compute), 1, &queuePriority});
			}
		}
		if (requestedQueueTypes & vk::QueueFlagBits::eTransfer)
		{
			if (m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Graphics &&
				m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Compute)
			{
				m_QueueCreateInfos.push_back({{}, static_cast<uint32>(m_QueueFamilyIndices.Transfer), 1, &queuePriority});
			}
		}

		m_DepthFormat = FindDepthFormat();
	}

	uint32 VulkanPhysicalDevice::GetMemoryTypeIndex(uint32 typeBits, vk::MemoryPropertyFlags properties) const
	{
		// Iterate over all memory types available for the device used in this example
		for (uint32 i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
		{
			if (typeBits & 1)
			{
				if ((m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}
			typeBits >>= 1;
		}

		NEO_CORE_ASSERT(false, "Could not find memory index");
		return UINT32_MAX;
	}

	VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::GetQueueFamilyIndices(vk::QueueFlags queueFlags)
	{
		QueueFamilyIndices indices;
		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if (queueFlags & vk::QueueFlagBits::eCompute)
		{
			for (uint32 i = 0; i < m_QueueFamilyProperties.size(); i++)
			{
				if ((m_QueueFamilyProperties[i].queueFlags & queueFlags) &&
					!(m_QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics))
				{
					indices.Compute = i;
					break;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if (queueFlags & vk::QueueFlagBits::eTransfer)
		{
			for (uint32 i = 0; i < m_QueueFamilyProperties.size(); i++)
			{
				if ((m_QueueFamilyProperties[i].queueFlags & queueFlags) &&
					!(m_QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
					!(m_QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute))
				{
					indices.Transfer = i;
					break;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32 i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			if (queueFlags & vk::QueueFlagBits::eTransfer && indices.Transfer == -1)
			{
				if (m_QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer)
				{
					indices.Transfer = i;
				}
			}

			if (queueFlags & vk::QueueFlagBits::eCompute && indices.Compute == -1)
			{
				if (m_QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute)
				{
					indices.Compute = i;
				}
			}

			if (queueFlags & vk::QueueFlagBits::eGraphics)
			{
				if (m_QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
				{
					indices.Graphics = i;
				}
			}
		}

		return indices;
	}

	vk::Format VulkanPhysicalDevice::FindDepthFormat()
	{
		// Since all depth formats may be optional, we need to find a suitable depth format to use
		// Start with the highest precision packed format
		std::vector<vk::Format> depthFormats = {vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint,
												vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm};

		for (auto& format : depthFormats)
		{
			vk::FormatProperties formatProps = m_Handle.getFormatProperties(format);
			// Format must support depth stencil attachment for optimal tiling
			if (formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
			{
				return format;
			}
		}
		return vk::Format::eUndefined;
	}

	VulkanDevice::VulkanDevice(SharedRef<VulkanPhysicalDevice>& physicalDevice)
		: m_PhysicalDevice(physicalDevice)
	{
		NEO_CORE_ASSERT(physicalDevice, "Vulkan device initialized with non existant physical device");

#ifdef NEO_DEBUG
		vk::DeviceCreateInfo deviceCreateInfo{{},
											  static_cast<uint32>(physicalDevice->m_QueueCreateInfos.size()),
											  physicalDevice->m_QueueCreateInfos.data(),
											  static_cast<uint32>(m_ValidationLayers.size()),
											  m_ValidationLayers.data(),
											  static_cast<uint32>(physicalDevice->m_RequiredPhysicalDeviceExtensions.size()),
											  physicalDevice->m_RequiredPhysicalDeviceExtensions.data(),
											  nullptr};
#else
		vk::DeviceCreateInfo deviceCreateInfo{{},
											  static_cast<uint32>(physicalDevice->m_QueueCreateInfos.size()),
											  physicalDevice->m_QueueCreateInfos.data(),
											  0,
											  nullptr,
											  static_cast<uint32>(physicalDevice->m_RequiredPhysicalDeviceExtensions.size()),
											  physicalDevice->m_RequiredPhysicalDeviceExtensions.data(),
											  nullptr};
#endif

		vk::PhysicalDeviceScalarBlockLayoutFeatures scalarLayoutFeatures;
		scalarLayoutFeatures.scalarBlockLayout = VK_TRUE;
		vk::PhysicalDeviceDescriptorIndexingFeatures descriptorFeatures;
		descriptorFeatures.runtimeDescriptorArray = VK_TRUE;
		descriptorFeatures.pNext = &scalarLayoutFeatures;
		vk::PhysicalDeviceFeatures deviceFeatures;
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.shaderClipDistance = VK_TRUE;
		vk::PhysicalDeviceFeatures2 deviceFeatures2;
		deviceFeatures2.pNext = &descriptorFeatures;
		deviceFeatures2.features = deviceFeatures;
		std::vector<vk::PhysicalDeviceFeatures> features = {deviceFeatures};

		deviceCreateInfo.pNext = &deviceFeatures2;
		m_Handle = physicalDevice->GetHandle().createDeviceUnique(deviceCreateInfo);

		vk::CommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Graphics;
		cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		m_CommandPool = m_Handle.get().createCommandPoolUnique(cmdPoolInfo);

		m_GraphicsQueue = m_Handle.get().getQueue(physicalDevice->m_QueueFamilyIndices.Graphics, 0);
	}

	vk::CommandBuffer VulkanDevice::GetCommandBuffer(bool begin)
	{
		vk::CommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.commandPool = m_CommandPool.get();
		cmdBufAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		cmdBufAllocateInfo.commandBufferCount = 1;

		vk::CommandBuffer cmdBuffer = m_Handle.get().allocateCommandBuffers(cmdBufAllocateInfo)[0];

		// If requested, also start the new command buffer
		if (begin)
		{
			vk::CommandBufferBeginInfo cmdBufferBeginInfo{};
			cmdBuffer.begin(cmdBufferBeginInfo);
		}

		return cmdBuffer;
	}

	void VulkanDevice::FlushCommandBuffer(vk::CommandBuffer commandBuffer)
	{
		const uint64 DEFAULT_FENCE_TIMEOUT = 100000000000;

		NEO_CORE_ASSERT(commandBuffer);

		commandBuffer.end();

		vk::SubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Create fence to ensure that the command buffer has finished executing
		vk::FenceCreateInfo fenceCreateInfo = {};
		vk::UniqueFence fence = m_Handle.get().createFenceUnique(fenceCreateInfo);

		// Submit to the queue
		m_GraphicsQueue.submit(submitInfo, fence.get());
		// Wait for the fence to signal that command buffer has finished executing
		m_Handle.get().waitForFences(fence.get(), VK_TRUE, DEFAULT_FENCE_TIMEOUT);

		m_Handle.get().freeCommandBuffers(m_CommandPool.get(), commandBuffer);
	}

	vk::CommandBuffer VulkanDevice::CreateSecondaryCommandBuffer()
	{
		vk::CommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.commandPool = m_CommandPool.get();
		cmdBufAllocateInfo.level = vk::CommandBufferLevel::eSecondary;
		cmdBufAllocateInfo.commandBufferCount = 1;

		vk::CommandBuffer cmdBuffer = m_Handle.get().allocateCommandBuffers(cmdBufAllocateInfo)[0];
		return cmdBuffer;
	}

	SharedRef<VulkanDevice> VulkanDevice::Create(SharedRef<VulkanPhysicalDevice>& physicalDevice)
	{
		return SharedRef<VulkanDevice>::Create(physicalDevice);
	}

} // namespace Neon
