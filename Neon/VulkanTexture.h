#pragma once

#include "Renderer/Texture.h"

namespace Neon
{
	class VulkanTexture2D : Texture2D
	{
		VulkanTexture2D(const std::string& path, bool srgb = false);
		VulkanTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap = TextureWrap::Clamp);
		virtual ~VulkanTexture2D();

		void Invalidate();

		virtual uint32_t GetWidth() const override
		{
			return m_Width;
		}
		virtual uint32_t GetHeight() const override
		{
			return m_Height;
		}

		virtual void Bind(uint32_t slot = 0) const override;

		const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const
		{
			return m_DescriptorImageInfo;
		}

		void Lock() override;

		void Unlock() override;

		void Resize(uint32_t width, uint32_t height) override;

		Buffer GetWriteableBuffer() override;

		bool Loaded() const override;

		const std::string& GetPath() const override;

		TextureFormat GetFormat() const override;

		uint32_t GetMipLevelCount() const override;

		RendererID GetRendererID() const override;

		bool operator==(const Texture& other) const override
		{
			throw std::logic_error("The method or operation is not implemented.");
		}

	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;

		Buffer m_ImageData;

		VkDeviceMemory m_DeviceMemory;
		VkImage m_Image;

		VkDescriptorImageInfo m_DescriptorImageInfo = {};
	};
}
