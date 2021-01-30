#pragma once

#include "Vulkan.h"
#include "VulkanDevice.h"

#include <string>

namespace Neon
{
	struct VulkanBuffer
	{
		uint32 Size;
		vk::UniqueDeviceMemory Memory;
		vk::UniqueBuffer Handle;
	};

	struct VulkanImage
	{
		uint32 Width;
		uint32 Height;
		vk::Format Format;
		vk::UniqueImage Image;
		vk::UniqueDeviceMemory DeviceMemory;
	};

	class VulkanAllocator
	{
	public:
		VulkanAllocator() = default;
		VulkanAllocator(const SharedRef<VulkanDevice>& device, const std::string& tag);
		~VulkanAllocator() = default;

		void Allocate(vk::MemoryRequirements requirements, vk::UniqueDeviceMemory& outDeviceMemory,
					  vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eDeviceLocal);

		void AllocateBuffer(VulkanBuffer& outBuffer, uint32 size, vk::BufferUsageFlagBits usage,
							vk::MemoryPropertyFlags memPropFlags);

		void UpdateBuffer(VulkanBuffer& outBuffer, const void* data, uint32 size = 0);

	private:
		std::string m_Tag;
		SharedRef<VulkanDevice> m_Device;
	};
} // namespace Neon
