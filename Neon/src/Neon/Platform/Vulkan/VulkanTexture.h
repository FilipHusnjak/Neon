#pragma once

#include "Neon/Platform/Vulkan/VulkanAllocator.h"
#include "Neon/Renderer/Texture.h"

namespace Neon
{
	static vk::Format ConvertTextureFormatToVulkanFormat(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::None:
				return vk::Format::eUndefined;
			case TextureFormat::RGBA:
				return vk::Format::eR8G8B8A8Unorm;
			case TextureFormat::SRGBA:
				return vk::Format::eR8G8B8A8Srgb;
			case TextureFormat::RGBAFloat32:
				return vk::Format::eR32G32B32A32Sfloat;
			default:
				NEO_CORE_ERROR("Unknown texture format!");
				return vk::Format::eUndefined;
		}
	}

	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(TextureType type);
		VulkanTexture2D(const std::string& path, TextureType type);

		virtual ~VulkanTexture2D();

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

	class VulkanTextureCube : public TextureCube
	{
	public:
		VulkanTextureCube(TextureType type);
		VulkanTextureCube(uint32 faceSize, TextureType type);
		VulkanTextureCube(const std::string& path, TextureType type);
		VulkanTextureCube(const std::array<std::string, 6>& paths, TextureType type);

		virtual ~VulkanTextureCube();

		bool Loaded() const override
		{
			return m_Data.Data != nullptr;
		}

		uint32 GetMipLevelCount() const override
		{
			return Texture::CalculateMipMapCount(m_Image.Width, m_Image.Height);
		}

		uint32 GetFaceSize() const override
		{
			return m_FaceSize;
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

		void GetFace(const byte* sourceData, byte* destData, uint32 xOffset, uint32 yOffset);
		void RotateFaceClockwise(byte* data);
		void RotateFaceCounterClockwise(byte* data);

	private:
		Buffer m_Data{};
		VulkanImage m_Image{};

		uint32 m_FaceSize;

		vk::ImageLayout m_Layout{};
		vk::UniqueSampler m_Sampler{};
		vk::UniqueImageView m_View{};

		VulkanAllocator m_Allocator{};
	};
} // namespace Neon
