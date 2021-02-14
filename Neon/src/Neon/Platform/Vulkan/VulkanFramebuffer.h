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
		const SharedRef<Texture2D>& GetSampledImage() const override
		{
			NEO_CORE_ASSERT(m_SampledImageIndex > -1);
			return m_Textures[m_SampledImageIndex];
		}

		void* GetHandle() const override
		{
			return m_Handle.get();
		}

	private:
		void Create(uint32 width, uint32 height);

	private:
		vk::UniqueFramebuffer m_Handle;

		std::vector<SharedRef<Texture2D>> m_Textures;

		vk::UniqueDescriptorPool m_ImGuiDescPool;

		vk::UniqueDescriptorPool m_DescPool;
		vk::UniqueDescriptorSetLayout m_ColorImageDescSetLayout;
		vk::UniqueDescriptorSet m_ColorImageDescSet;

		int32 m_SampledImageIndex = -1;
	};
} // namespace Neon
