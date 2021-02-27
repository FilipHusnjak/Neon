#pragma once

#include "Neon/Platform/Vulkan/VulkanAllocator.h"
#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "Neon/Renderer/Texture.h"

namespace Neon
{
	static vk::Format ConvertNeonTextureFormatToVulkanFormat(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::None:
				return vk::Format::eUndefined;
			case TextureFormat::RGBA8:
				return vk::Format::eR8G8B8A8Unorm;
			case TextureFormat::SRGBA8:
				return vk::Format::eR8G8B8A8Srgb;
			case TextureFormat::RGBA16F:
				return vk::Format::eR16G16B16A16Sfloat;
			case TextureFormat::RGBA32F:
				return vk::Format::eR32G32B32A32Sfloat;
			case TextureFormat::Depth:
				const auto& device = VulkanContext::GetDevice();
				return device->GetPhysicalDevice()->GetDepthFormat();
		}
		NEO_CORE_ASSERT(false, "Uknown texture format!");
		return vk::Format::eUndefined;
	}

	static vk::SamplerAddressMode ConvertNeonTextureWrapToVulkanTextureWrap(TextureWrap wrap)
	{
		switch (wrap)
		{
			case TextureWrap::None:
			case TextureWrap::Repeat:
				return vk::SamplerAddressMode::eRepeat;
			case TextureWrap::Clamp:
				return vk::SamplerAddressMode::eClampToEdge;
		}
		NEO_CORE_ASSERT(false, "Uknown texture wrap!");
		return vk::SamplerAddressMode::eRepeat;
	}

	static vk::Filter ConvertNeonTextureMinMagFilterToVulkanMinMagFilter(TextureMinMagFilter format)
	{
		switch (format)
		{
			case TextureMinMagFilter::Linear:
				return vk::Filter::eLinear;
			case TextureMinMagFilter::Nearest:
				return vk::Filter::eNearest;
		}
		NEO_CORE_ASSERT(false, "Uknown texture min mag filter!");
		return vk::Filter::eLinear;
	}

	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(const TextureSpecification& specification);
		VulkanTexture2D(const std::string& path, const TextureSpecification& specification);

		virtual ~VulkanTexture2D();

		bool Loaded() const override
		{
			return m_Data.Data != nullptr;
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

		vk::ImageView GetView(uint32 mip)
		{
			NEO_CORE_ASSERT(mip < m_Views.size());
			return m_Views[mip].get();
		}

		vk::DescriptorImageInfo GetTextureDescription(uint32 mipLevel) const
		{
			NEO_CORE_ASSERT(mipLevel < m_MipLevelCount);
			return {m_Sampler.get(), m_Views[mipLevel].get(), m_Layout};
		}

		void* GetRendererId() const override;

		void RegenerateMipMaps() override;

	private:
		void CreateDefault();
		void Invalidate();
		void Update();
		void CreateRendererId();

	private:
		Buffer m_Data{};
		VulkanImage m_Image{};

		vk::ImageLayout m_Layout{};
		vk::UniqueSampler m_Sampler{};
		std::vector<vk::UniqueImageView> m_Views;

		VulkanAllocator m_Allocator{};

		vk::UniqueDescriptorPool m_DescPool;
		vk::UniqueDescriptorSetLayout m_DescSetLayout;
		vk::UniqueDescriptorSet m_DescSet;
	};

	class VulkanTextureCube : public TextureCube
	{
	public:
		VulkanTextureCube(const TextureSpecification& specification);
		VulkanTextureCube(const std::string& path, const TextureSpecification& specification);
		VulkanTextureCube(const std::array<std::string, 6>& paths, const TextureSpecification& specification);

		virtual ~VulkanTextureCube();

		bool Loaded() const override
		{
			return m_Data.Data != nullptr;
		}

		uint32 GetFaceSize() const override
		{
			return m_FaceSize;
		}

		Buffer GetTextureData() override
		{
			return m_Data;
		}

		vk::DescriptorImageInfo GetTextureDescription(uint32 mipLevel) const
		{
			NEO_CORE_ASSERT(mipLevel < m_MipLevelCount);
			return {m_Sampler.get(), m_Views[mipLevel].get(), m_Layout};
		}

		void RegenerateMipMaps() override;

	private:
		void Invalidate();
		void CreateDefault();

		void GetFace(const byte* sourceData, byte* destData, uint32 xOffset, uint32 yOffset);
		void RotateFaceClockwise(byte* data);
		void RotateFaceCounterClockwise(byte* data);

	private:
		Buffer m_Data{};
		VulkanImage m_Image{};

		uint32 m_FaceSize;

		vk::ImageLayout m_Layout{};
		vk::UniqueSampler m_Sampler{};
		std::vector<vk::UniqueImageView> m_Views;

		VulkanAllocator m_Allocator{};
	};
} // namespace Neon
