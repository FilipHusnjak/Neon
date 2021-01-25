#pragma once

#include "Renderer/Framebuffer.h"
#include "Vulkan.h"

namespace Neon
{
	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		~VulkanFramebuffer() = default;
		void Resize(uint32 width, uint32 height, bool forceRecreate = false) override;

		void* GetSampledImageId() const override
		{
			return m_ColorImageDescSet.get();
		}

		void* GetHandle() const override
		{
			return m_Handle.get();
		}

		vk::ImageView GetSampledImageView(uint32 index);

	private:
		void Create(uint32 width, uint32 height);

	private:
		vk::UniqueFramebuffer m_Handle;

		struct FrameBufferAttachment
		{
			vk::UniqueImage Image;
			vk::UniqueDeviceMemory Memory;
			vk::UniqueImageView View;
		};

		std::vector<FrameBufferAttachment> m_Attachments;

		vk::UniqueDescriptorPool m_ImGuiDescPool;

		vk::UniqueDescriptorPool m_DescPool;
		vk::UniqueDescriptorSetLayout m_ColorImageDescSetLayout;
		vk::UniqueDescriptorSet m_ColorImageDescSet;

		vk::UniqueSampler m_AttachmentSampler;
		vk::DescriptorImageInfo m_AttachmentDescriptorInfo;
	};
} // namespace Neon
