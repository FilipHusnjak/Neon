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
			NEO_CORE_ASSERT(m_SampledImageIndex > -1 && m_SampledImageIndex < m_Textures.size());
			return m_Textures[m_SampledImageIndex]->GetRendererId();
		}
		const SharedRef<Texture2D>& GetSampledImage() const override
		{
			NEO_CORE_ASSERT(m_SampledImageIndex > -1 && m_SampledImageIndex < m_Textures.size());
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

		int32 m_SampledImageIndex = -1;
	};
} // namespace Neon
