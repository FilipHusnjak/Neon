#pragma once

#include "Neon/Platform/Vulkan/Vulkan.h"
#include "Renderer/RenderPass.h"

namespace Neon
{
	static vk::SampleCountFlagBits ConvertSampleCountToVulkan(uint32 samples)
	{
		switch (samples)
		{
			case 1:
				return vk::SampleCountFlagBits::e1;
			case 2:
				return vk::SampleCountFlagBits::e2;
			case 4:
				return vk::SampleCountFlagBits::e4;
			case 8:
				return vk::SampleCountFlagBits::e8;
			case 16:
				return vk::SampleCountFlagBits::e16;
			case 32:
				return vk::SampleCountFlagBits::e32;
			case 64:
				return vk::SampleCountFlagBits::e64;
		}
		NEO_CORE_ASSERT(false, "Uknown sample count!");
		return vk::SampleCountFlagBits::e1;
	}

	static vk::AttachmentLoadOp ConvertLoadOpToVulkan(AttachmentLoadOp loadOp)
	{
		switch (loadOp)
		{
			case AttachmentLoadOp::DontCare:
				return vk::AttachmentLoadOp::eDontCare;
			case AttachmentLoadOp::Clear:
				return vk::AttachmentLoadOp::eClear;
			case AttachmentLoadOp::Load:
				return vk::AttachmentLoadOp::eLoad;
		}
		NEO_CORE_ASSERT(false, "Uknown load op!");
		return vk::AttachmentLoadOp::eDontCare;
	}

	static vk::AttachmentStoreOp ConvertStoreOpToVulkan(AttachmentStoreOp storeOp)
	{
		switch (storeOp)
		{
			case AttachmentStoreOp::DontCare:
				return vk::AttachmentStoreOp::eDontCare;
			case AttachmentStoreOp::Store:
				return vk::AttachmentStoreOp::eStore;
		}
		NEO_CORE_ASSERT(false, "Uknown store op!");
		return vk::AttachmentStoreOp::eDontCare;
	}

	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(RenderPassSpecification specification);
		~VulkanRenderPass() = default;

		void* GetHandle() const override
		{
			return m_Handle.get();
		}

	private:
		vk::UniqueRenderPass m_Handle;
	};
} // namespace Neon
