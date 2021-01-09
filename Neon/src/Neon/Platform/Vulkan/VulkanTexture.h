#pragma once

#include "Renderer/Texture.h"
#include "VulkanAllocator.h"

namespace Neon
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const std::string& path, bool srgb = false);
		VulkanTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap = TextureWrap::Clamp);

		virtual ~VulkanTexture2D() = default;

		void Resize(uint32_t width, uint32_t height) override;

		bool Loaded() const override
		{
			return m_Data.Data != nullptr;
		}

		uint32 GetMipLevelCount() const override
		{
			return Texture::CalculateMipMapCount(m_Image.Width, m_Image.Height);
		}

		uint32 GetWidth() const override
		{
			return m_Image.Width;
		}
		uint32 GetHeight() const override
		{
			return m_Image.Height;
		}

		Buffer GetTextureData() override
		{
			return m_Data;
		}

		vk::DescriptorImageInfo GetTextureDescription() const
		{
			return {m_Sampler.get(), m_View.get(), m_Layout};
		}

		bool operator==(const Texture& other) const override
		{
			throw std::logic_error("The method or operation is not implemented.");
		}

	private:
		Buffer m_Data{};
		VulkanImage m_Image{};

		vk::ImageLayout m_Layout{};
		vk::UniqueSampler m_Sampler{};
		vk::UniqueImageView m_View{};

		VulkanAllocator m_Allocator{};
	};
} // namespace Neon
