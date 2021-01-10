#include "neopch.h"

#include "VulkanContext.h"
#include "VulkanTexture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Neon
{
	VulkanTexture2D::VulkanTexture2D(bool srgb /*= false*/)
	{
		CreateDefaultTexture();
	}

	VulkanTexture2D::VulkanTexture2D(const std::string& path, bool srgb /*= false*/)
		: Texture2D(path)
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "Texture2D");

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		m_Data.Data = stbi_load(path.c_str(), &width, &height, &channels, 4);
		m_Data.Size = width * height * 4;
		if (!m_Data.Data)
		{
			NEO_CORE_WARN("Failed to load texture: {0}", path);
			CreateDefaultTexture();
		}
		else
		{
			m_Image.Width = width;
			m_Image.Height = height;

			Invalidate();
		}		
	}

	void VulkanTexture2D::Invalidate()
	{
		auto device = VulkanContext::GetDevice();
		auto deviceHandle = device->GetHandle();

		uint32 size = m_Data.Size;

		vk::Format format = vk::Format::eR8G8B8A8Unorm;

		vk::MemoryAllocateInfo memAllocInfo{};

		// Copy data to an optimal tiled image
		// This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

		// Create a host-visible staging buffer that contains the raw image data
		// This buffer will be the data source for copying texture data to the optimal tiled image on the device
		VulkanBuffer stagingBuffer;
		m_Allocator.AllocateBuffer(stagingBuffer, size, vk::BufferUsageFlagBits::eTransferSrc,
								   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		// Copy texture data into host local staging buffer
		m_Allocator.UpdateBuffer(stagingBuffer, m_Data.Data);

		/*
		// Setup buffer copy regions for each mip level
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t i = 0; i < texture.mipLevels; i++) {
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[i].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[i].extent().y);
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			offset += static_cast<uint32_t>(tex2D[i].size());
		}
		*/

		vk::BufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = m_Image.Width;
		bufferCopyRegion.imageExtent.height = m_Image.Height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;

		// Create optimal tiled target image on the device
		vk::ImageCreateInfo imageCreateInfo{};
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		// Set initial layout of the image to undefined
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.extent = {m_Image.Width, m_Image.Height, 1};
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		m_Image.Image = deviceHandle.createImageUnique(imageCreateInfo);

		vk::MemoryRequirements memoryRequirements = deviceHandle.getImageMemoryRequirements(m_Image.Image.get());
		m_Allocator.Allocate(memoryRequirements, m_Image.DeviceMemory, vk::MemoryPropertyFlagBits::eDeviceLocal);
		deviceHandle.bindImageMemory(m_Image.Image.get(), m_Image.DeviceMemory.get(), 0);

		vk::CommandBuffer copyCmd = device->GetCommandBuffer(true);

		// Image memory barriers for the texture image

		// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
		vk::ImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		// We will transition on all mip levels
		subresourceRange.levelCount = 1; // TODO: Support mips
		// The 2D texture only has one layer
		subresourceRange.layerCount = 1;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_Image.Image.get();
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
		// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
		// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0,
								nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		// Copy mip levels from staging buffer
		copyCmd.copyBufferToImage(stagingBuffer.Handle.get(), m_Image.Image.get(), vk::ImageLayout::eTransferDstOptimal, 1,
								  &bufferCopyRegion);

		// Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
		// Source pipeline stage stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
								vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		// Store current layout for later reuse
		m_Layout = imageMemoryBarrier.newLayout;

		device->FlushCommandBuffer(copyCmd);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CREATE TEXTURE SAMPLER
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create a texture sampler
		// In Vulkan textures are accessed by samplers
		// This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
		// Note: Similar to the samplers available with OpenGL 3.3
		vk::SamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.minFilter = vk::Filter::eLinear;
		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.compareOp = vk::CompareOp::eNever;
		samplerCreateInfo.minLod = 0.0f;
		// Set max level-of-detail to mip level count of the texture
		samplerCreateInfo.maxLod = 1.0f;
		// Enable anisotropic filtering
		// This feature is optional, so we must check if it's supported on the device

		// TODO:
		/*if (vulkanDevice->features.samplerAnisotropy) {
			// Use max. level of anisotropy for this example
			sampler.maxAnisotropy = 1.0f;// vulkanDevice->properties.limits.maxSamplerAnisotropy;
			sampler.anisotropyEnable = VK_TRUE;
		}
		else {
			// The device does not support anisotropic filtering
			sampler.maxAnisotropy = 1.0;
			sampler.anisotropyEnable = VK_FALSE;
		}*/
		samplerCreateInfo.maxAnisotropy = 1.0;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		m_Sampler = deviceHandle.createSamplerUnique(samplerCreateInfo);

		// Create image view
		// Textures are not directly accessed by the shaders and
		// are abstracted by image views containing additional
		// information and sub resource ranges
		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB,
										  vk::ComponentSwizzle::eA};
		// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
		// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		// Linear tiling usually won't support mip maps
		// Only set mip map count if optimal tiling is used
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		// The view will be based on the texture's image
		imageViewCreateInfo.image = m_Image.Image.get();

		m_View = deviceHandle.createImageViewUnique(imageViewCreateInfo);
	}

	void VulkanTexture2D::CreateDefaultTexture()
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "Texture2D");

		int width, height, channels;
		width = height = 1;
		channels = 4;
		auto* color = new glm::u8vec4(255, 0, 255, 255);
		m_Data.Data = reinterpret_cast<stbi_uc*>(color);
		m_Data.Size = width * height * 4;
		
		m_Image.Width = width;
		m_Image.Height = height;

		Invalidate();
	}

} // namespace Neon
