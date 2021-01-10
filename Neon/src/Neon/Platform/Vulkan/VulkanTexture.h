#pragma once

#include "Renderer/Texture.h"
#include "VulkanAllocator.h"

namespace Neon
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(bool srgb = false);
		VulkanTexture2D(const std::string& path, bool srgb = false);

		virtual ~VulkanTexture2D() = default;

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
		void Invalidate();
		void CreateDefaultTexture();

	private:
		Buffer m_Data{};
		VulkanImage m_Image{};

		vk::ImageLayout m_Layout{};
		vk::UniqueSampler m_Sampler{};
		vk::UniqueImageView m_View{};

		VulkanAllocator m_Allocator{};
	};
} // namespace Neon
