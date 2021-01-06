#include "neopch.h"

#include "VulkanAllocator.h"
#include "VulkanContext.h"

namespace Neon
{
	VulkanAllocator::VulkanAllocator(const SharedRef<VulkanDevice>& device, const std::string& tag)
		: m_Tag(tag)
		, m_Device(device)
	{
	}

	void VulkanAllocator::Allocate(vk::MemoryRequirements requirements, vk::UniqueDeviceMemory& outDeviceMemory,
								   vk::MemoryPropertyFlags flags /*= vk::MemoryPropertyFlagBits::eDeviceLocal*/)
	{
		NEO_CORE_ASSERT(m_Device, "Device not initialized!");
		NEO_CORE_TRACE("VulkanAllocator ({0}): allocating {1} bytes", m_Tag, requirements.size);

		vk::MemoryAllocateInfo memAlloc = {};
		memAlloc.allocationSize = requirements.size;
		memAlloc.memoryTypeIndex = m_Device->GetPhysicalDevice()->GetMemoryTypeIndex(requirements.memoryTypeBits, flags);
		outDeviceMemory = m_Device->GetHandle().allocateMemoryUnique(memAlloc);
	}

	void VulkanAllocator::AllocateBuffer(VulkanBuffer& outBuffer, uint32 size, vk::BufferUsageFlagBits usage,
										 vk::MemoryPropertyFlags memPropFlags)
	{
		NEO_CORE_ASSERT(m_Device, "Device not initialized!");

		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		outBuffer.Handle = m_Device->GetHandle().createBufferUnique(bufferInfo);

		vk::MemoryRequirements memRequirements = m_Device->GetHandle().getBufferMemoryRequirements(outBuffer.Handle.get());
		Allocate(memRequirements, outBuffer.Memory, memPropFlags);

		m_Device->GetHandle().bindBufferMemory(outBuffer.Handle.get(), outBuffer.Memory.get(), 0);
		outBuffer.Size = size;
	}

	void VulkanAllocator::UpdateBuffer(VulkanBuffer& outBuffer, const void* data)
	{
		NEO_CORE_ASSERT(m_Device, "Device not initialized!");

		void* dest;
		m_Device->GetHandle().mapMemory(outBuffer.Memory.get(), 0, outBuffer.Size, vk::MemoryMapFlags(), &dest);
		memcpy(dest, data, outBuffer.Size);
		m_Device->GetHandle().unmapMemory(outBuffer.Memory.get());
	}

} // namespace Neon
