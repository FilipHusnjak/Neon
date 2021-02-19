#include "neopch.h"

#include "Neon/Core/ThreadPool.h"
#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "Neon/Platform/Vulkan/VulkanRenderPass.h"
#include "VulkanTexture.h"

#include <softfloat/softfloat.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Neon
{
	static void GenerateMipMaps(vk::CommandBuffer vulkanCommandBuffer, const VulkanImage& image,
								vk::ImageMemoryBarrier& imageMemoryBarrier)
	{
		uint32 mipCount = imageMemoryBarrier.subresourceRange.levelCount;
		imageMemoryBarrier.subresourceRange.levelCount = 1;

		uint32 mipWidth = image.Width;
		uint32 mipHeight = image.Height;
		for (uint32 i = 1; i < mipCount; i++)
		{
			imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;

			vulkanCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
												vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			vk::ImageBlit blit{};
			blit.srcOffsets[0] = {0, 0, 0};
			blit.srcOffsets[1] = {static_cast<int32>(mipWidth), static_cast<int32>(mipHeight), 1};
			blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = imageMemoryBarrier.subresourceRange.layerCount;
			blit.dstOffsets[0] = {0, 0, 0};
			blit.dstOffsets[1] = {mipWidth > 1 ? static_cast<int32>(mipWidth) / 2 : 1,
								  mipHeight > 1 ? static_cast<int32>(mipHeight) / 2 : 1, 1};
			blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = imageMemoryBarrier.subresourceRange.layerCount;

			vulkanCommandBuffer.blitImage(image.Handle.get(), vk::ImageLayout::eTransferSrcOptimal, image.Handle.get(),
										  vk::ImageLayout::eTransferDstOptimal, 1, &blit, vk::Filter::eLinear);

			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
			imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			imageMemoryBarrier.newLayout = vk::ImageLayout::eGeneral;

			vulkanCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
												vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			if (mipWidth > 1)
			{
				mipWidth /= 2;
			}
			if (mipHeight > 1)
			{
				mipHeight /= 2;
			}
		}

		imageMemoryBarrier.subresourceRange.baseMipLevel = mipCount - 1;
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eGeneral;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
		// Source pipeline stage stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
		vulkanCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
											vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& specification)
		: Texture2D(specification)
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "Texture2D");

		CreateDefaultTexture();

		CreateRendererId();
	}

	VulkanTexture2D::VulkanTexture2D(const std::string& path, const TextureSpecification& specification)
		: Texture2D(path, specification)
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "Texture2D");

		int width, height, channels;

		if (stbi_is_hdr(path.c_str()))
		{
			stbi_set_flip_vertically_on_load(false);
			float* data = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb);

			m_Format = TextureFormat::RGBA16F;

			if (data)
			{
				auto* newData = new uint16[width * (size_t)height * (size_t)GetBytesPerPixel(m_Format)];
				{
					ThreadPool pool;

					auto task = [data, newData, width](uint32 low, uint32 high) {
						for (uint32 y = low; y < high; y++)
						{
							float* src = data + 3 * (size_t)width * y;
							uint16* dest = newData + 4 * (size_t)width * y;
							for (uint32 x = 0; x < static_cast<uint32>(width); x++)
							{
								dest[4 * x] = float_to_sf16(src[3 * x], SF_NEARESTEVEN);
								dest[4 * x + 1] = float_to_sf16(src[3 * x + 1], SF_NEARESTEVEN);
								dest[4 * x + 2] = float_to_sf16(src[3 * x + 2], SF_NEARESTEVEN);
							}
						}
					};

					uint32 step = height / pool.GetThreadCount();
					for (uint32 i = 0; i < static_cast<uint32>(height); i += step)
					{
						pool.QueueTask(task, i, std::min((uint32)height - 1, i + step));
					}
				}

				stbi_image_free(data);

				m_Data.Data = (byte*)newData;
			}
		}
		else
		{
			stbi_set_flip_vertically_on_load(true);
			m_Data.Data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

			m_Format = specification.Type == TextureType::SRGB ? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
		}

		if (!m_Data.Data)
		{
			NEO_CORE_WARN("Failed to load texture: {0}", path);
			CreateDefaultTexture();
		}
		else
		{
			m_Image.Width = width;
			m_Image.Height = height;

			uint32 bytesPerPixel = GetBytesPerPixel(m_Format);

			m_Data.Size = width * height * bytesPerPixel;

			m_MipLevelCount = CalculateMaxMipMapCount(m_Image.Width, m_Image.Height);

			CreateResources(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
							vk::ImageUsageFlagBits::eSampled);
			Update();
		}

		CreateRendererId();
	}

	VulkanTexture2D::VulkanTexture2D(TextureFormat format, uint32 width, uint32 height, uint32 sampleCount,
									 vk::ImageUsageFlags usage)
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "Texture2D");

		m_Format = format;
		m_Image.Width = width;
		m_Image.Height = height;
		m_Specification.SampleCount = sampleCount;
		m_Layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_MipLevelCount = 1;

		CreateResources(usage);

		if (usage & vk::ImageUsageFlagBits::eSampled)
		{
			CreateRendererId();
		}
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		if (m_Data.Data)
		{
			delete[] m_Data.Data;
		}
	}

	void* VulkanTexture2D::GetRendererId() const
	{
		return m_DescSet.get();
	}

	void VulkanTexture2D::RegenerateMipMaps()
	{
		NEO_CORE_ASSERT(m_Format != TextureFormat::Depth);

		auto& commandBuffer = VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Graphics, true);
		vk::CommandBuffer vulkanCommandBuffer = (VkCommandBuffer)commandBuffer->GetHandle();

		// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
		vk::ImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		// We will transition on all mip levels
		subresourceRange.levelCount = m_MipLevelCount;
		// The 2D texture only has one layer
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_Image.Handle.get();
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;

		vulkanCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer,
											vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		GenerateMipMaps(vulkanCommandBuffer, m_Image, imageMemoryBarrier);

		VulkanContext::Get()->SubmitCommandBuffer(commandBuffer);
	}

	void VulkanTexture2D::CreateResources(vk::ImageUsageFlags usage)
	{
		auto device = VulkanContext::GetDevice();
		auto deviceHandle = device->GetHandle();

		// Create optimal tiled target image on the device
		vk::ImageCreateInfo imageCreateInfo{};
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.format = ConvertTextureFormatToVulkanFormat(m_Format);
		imageCreateInfo.mipLevels = m_MipLevelCount;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = ConvertSampleCountToVulkan(m_Specification.SampleCount);
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		// Set initial layout of the image to undefined
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.extent = {m_Image.Width, m_Image.Height, 1};
		imageCreateInfo.usage = usage;
		m_Image.Handle = deviceHandle.createImageUnique(imageCreateInfo);

		vk::MemoryRequirements memoryRequirements = deviceHandle.getImageMemoryRequirements(m_Image.Handle.get());
		m_Allocator.Allocate(memoryRequirements, m_Image.DeviceMemory, vk::MemoryPropertyFlagBits::eDeviceLocal);
		deviceHandle.bindImageMemory(m_Image.Handle.get(), m_Image.DeviceMemory.get(), 0);

		// Create a texture sampler
		// In Vulkan textures are accessed by samplers
		// This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
		// Note: Similar to the samplers available with OpenGL 3.3
		vk::SamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.minFilter = vk::Filter::eLinear;
		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerCreateInfo.addressModeU = ConvertTextureWrapToVulkanTextureWrap(m_Specification.Wrap);
		samplerCreateInfo.addressModeV = ConvertTextureWrapToVulkanTextureWrap(m_Specification.Wrap);
		samplerCreateInfo.addressModeW = ConvertTextureWrapToVulkanTextureWrap(m_Specification.Wrap);
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		// Set max level-of-detail to mip level count of the texture
		samplerCreateInfo.maxLod = static_cast<float>(m_MipLevelCount);
		samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;

		// Enable anisotropic filtering
		// This feature is optional, so we must check if it's supported on the device
		if (device->GetPhysicalDevice()->GetSupportedFeatures().samplerAnisotropy)
		{
			// Use max. level of anisotropy for this example
			samplerCreateInfo.maxAnisotropy = device->GetPhysicalDevice()->GetProperties().limits.maxSamplerAnisotropy;
			samplerCreateInfo.anisotropyEnable = VK_TRUE;
		}
		else
		{
			// The device does not support anisotropic filtering
			samplerCreateInfo.maxAnisotropy = 1.0;
			samplerCreateInfo.anisotropyEnable = VK_FALSE;
		}
		m_Sampler = deviceHandle.createSamplerUnique(samplerCreateInfo);

		// Create image view
		// Textures are not directly accessed by the shaders and
		// are abstracted by image views containing additional
		// information and sub resource ranges
		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = ConvertTextureFormatToVulkanFormat(m_Format);
		imageViewCreateInfo.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB,
										  vk::ComponentSwizzle::eA};
		// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
		// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
		imageViewCreateInfo.subresourceRange.aspectMask = m_Format == TextureFormat::Depth
															  ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil
															  : vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		// The view will be based on the texture's image
		imageViewCreateInfo.image = m_Image.Handle.get();

		// Linear tiling usually won't support mip maps
		// Only set mip map count if optimal tiling is used
		for (uint32 i = 0; i < m_MipLevelCount; i++)
		{
			imageViewCreateInfo.subresourceRange.levelCount = m_MipLevelCount - i;
			imageViewCreateInfo.subresourceRange.baseMipLevel = i;
			m_Views.push_back(deviceHandle.createImageViewUnique(imageViewCreateInfo));
		}
	}

	void VulkanTexture2D::Update()
	{
		auto device = VulkanContext::GetDevice();
		auto deviceHandle = device->GetHandle();

		NEO_CORE_ASSERT(m_Format != TextureFormat::Depth);

		uint32 size = m_Data.Size;

		uint32 bytesPerPixel = GetBytesPerPixel(m_Format);

		// Copy data to an optimal tiled image

		// Create a host-visible staging buffer that contains the raw image data
		// This buffer will be the data source for copying texture data to the optimal tiled image on the device
		VulkanBuffer stagingBuffer;
		m_Allocator.AllocateBuffer(stagingBuffer, size, vk::BufferUsageFlagBits::eTransferSrc,
								   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		// Copy texture data into host local staging buffer
		m_Allocator.UpdateBuffer(stagingBuffer, m_Data.Data);

		vk::BufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = m_Image.Width;
		bufferCopyRegion.imageExtent.height = m_Image.Height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;

		auto& commandBuffer = VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Graphics, true);
		vk::CommandBuffer copyCmd = (VkCommandBuffer)commandBuffer->GetHandle();

		// Image memory barriers for the texture image

		// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
		vk::ImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		// We will transition on all mip levels
		subresourceRange.levelCount = m_MipLevelCount;
		// The 2D texture only has one layer
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_Image.Handle.get();
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
		copyCmd.copyBufferToImage(stagingBuffer.Handle.get(), m_Image.Handle.get(), vk::ImageLayout::eTransferDstOptimal, 1,
								  &bufferCopyRegion);

		GenerateMipMaps(copyCmd, m_Image, imageMemoryBarrier);

		// Store current layout for later reuse
		m_Layout = imageMemoryBarrier.newLayout;

		VulkanContext::Get()->SubmitCommandBuffer(commandBuffer);
	}

	void VulkanTexture2D::CreateDefaultTexture()
	{
		m_Format = TextureFormat::RGBA8;

		int width = 1, height = 1;
		auto* color = new glm::u8vec4(255, 0, 255, 255);
		m_Data.Data = reinterpret_cast<stbi_uc*>(color);
		m_Data.Size = width * height * GetBytesPerPixel(m_Format);

		m_Image.Width = width;
		m_Image.Height = height;

		m_MipLevelCount = 1;

		CreateResources(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
						vk::ImageUsageFlagBits::eSampled);
		Update();
	}

	void VulkanTexture2D::CreateRendererId()
	{
		if (m_Format == TextureFormat::Depth)
		{
			return;
		}

		const auto device = VulkanContext::GetDevice();

		vk::DescriptorPoolSize poolSize = {vk::DescriptorType::eCombinedImageSampler, 1};
		vk::DescriptorPoolCreateInfo descPoolCreateInfo = {};
		descPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		descPoolCreateInfo.maxSets = 1;
		descPoolCreateInfo.poolSizeCount = 1;
		descPoolCreateInfo.pPoolSizes = &poolSize;
		m_DescPool = device->GetHandle().createDescriptorPoolUnique(descPoolCreateInfo);

		vk::DescriptorSetLayoutBinding binding = {};
		binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		binding.descriptorCount = 1;
		binding.stageFlags = vk::ShaderStageFlagBits::eFragment;

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
		descriptorSetLayoutCreateInfo.bindingCount = 1;
		descriptorSetLayoutCreateInfo.pBindings = &binding;
		m_DescSetLayout = device->GetHandle().createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);

		vk::DescriptorSetAllocateInfo descAllocInfo = {};
		descAllocInfo.descriptorPool = m_DescPool.get();
		descAllocInfo.descriptorSetCount = 1;
		descAllocInfo.pSetLayouts = &m_DescSetLayout.get();
		m_DescSet = std::move(device->GetHandle().allocateDescriptorSetsUnique(descAllocInfo)[0]);

		vk::DescriptorImageInfo imageInfo = GetTextureDescription(0);
		vk::WriteDescriptorSet descWrite = {};
		descWrite.dstSet = m_DescSet.get();
		descWrite.descriptorCount = 1;
		descWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descWrite.pImageInfo = &imageInfo;

		device->GetHandle().updateDescriptorSets({descWrite}, nullptr);
	}

	VulkanTextureCube::VulkanTextureCube(const TextureSpecification& specification)
		: TextureCube(specification)
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "TextureCube");

		CreateDefaultTexture();
	}

	VulkanTextureCube::VulkanTextureCube(uint32 faceSize, const TextureSpecification& specification)
		: TextureCube(specification)
		, m_FaceSize(faceSize)
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "TextureCube");

		switch (specification.Type)
		{
			case TextureType::RGB:
				m_Format = TextureFormat::RGBA8;
				break;
			case TextureType::SRGB:
				m_Format = TextureFormat::SRGBA8;
				break;
			case TextureType::HDR:
				m_Format = TextureFormat::RGBA16F;
				break;
			default:
				m_Format = TextureFormat::RGBA8;
				NEO_CORE_ASSERT(false, "Unknown texture type!");
				break;
		}

		m_Data.Size = 6 * m_FaceSize * m_FaceSize * GetBytesPerPixel(m_Format);
		m_Data.Data = new byte[m_Data.Size];

		Invalidate();
	}

	VulkanTextureCube::VulkanTextureCube(const std::string& path, const TextureSpecification& specification)
		: TextureCube(path, specification)
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "TextureCube");

		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);

		byte* data;
		if (stbi_is_hdr(path.c_str()))
		{
			data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

			m_Format = TextureFormat::RGBA16F;
		}
		else
		{
			data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

			m_Format = specification.Type == TextureType::SRGB ? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
		}

		if (!data)
		{
			NEO_CORE_WARN("Failed to load texture cube: {0}", path);
			CreateDefaultTexture();
		}
		else
		{
			uint32 bytesPerPixel = GetBytesPerPixel(m_Format);

			m_FaceSize = width / 4;
			NEO_CORE_ASSERT(m_FaceSize == height / 3, "Non-square faces!");

			m_Data.Size = 6 * m_FaceSize * m_FaceSize * bytesPerPixel;
			m_Data.Data = new byte[m_Data.Size];

			size_t faceOffset = static_cast<size_t>(m_FaceSize) * m_FaceSize * bytesPerPixel;

			// Front
			GetFace(data, m_Data.Data, m_FaceSize, m_FaceSize);
			// Back
			GetFace(data, m_Data.Data + faceOffset, 3 * m_FaceSize, m_FaceSize);
			// Top
			GetFace(data, m_Data.Data + 2 * faceOffset, m_FaceSize, 0);
			RotateFaceCounterClockwise(m_Data.Data + 2 * faceOffset);
			// Bottom
			GetFace(data, m_Data.Data + 3 * faceOffset, m_FaceSize, 2 * m_FaceSize);
			RotateFaceClockwise(m_Data.Data + 3 * faceOffset);
			// Left
			GetFace(data, m_Data.Data + 4 * faceOffset, 0, m_FaceSize);
			// Right
			GetFace(data, m_Data.Data + 5 * faceOffset, 2 * m_FaceSize, m_FaceSize);

			stbi_image_free(data);

			Invalidate();
		}
	}

	VulkanTextureCube::VulkanTextureCube(const std::array<std::string, 6>& paths, const TextureSpecification& specification)
		: TextureCube(paths, specification)
	{
		m_Allocator = VulkanAllocator(VulkanContext::GetDevice(), "TextureCube");

		stbi_set_flip_vertically_on_load(false);

		m_Data.Size = 0;

		uint32 offset = 0;
		uint32 bytesPerPixel = 4;
		for (auto& path : paths)
		{
			int width, height, channels;
			byte* data;
			if (stbi_is_hdr(path.c_str()))
			{
				data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

				m_Format = TextureFormat::RGBA16F;
			}
			else
			{
				data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

				m_Format = specification.Type == TextureType::SRGB ? TextureFormat::SRGBA8 : TextureFormat::RGBA8;
			}

			NEO_CORE_ASSERT(width == height, "Non-square faces!");

			if (!data)
			{
				NEO_CORE_WARN("Failed to load cube map, loading default one...");
				delete[] m_Data.Data;
				CreateDefaultTexture();
				return;
			}

			if (m_Data.Size == 0)
			{
				bytesPerPixel = GetBytesPerPixel(m_Format);

				m_FaceSize = width;

				m_Data.Size = 6 * m_FaceSize * m_FaceSize * bytesPerPixel;
				m_Data.Data = new byte[m_Data.Size];
			}

			NEO_CORE_ASSERT(m_FaceSize == width, "All cubemap images have to be the same size!");

			memcpy(m_Data.Data + offset, data, static_cast<size_t>(m_FaceSize) * m_FaceSize * bytesPerPixel);

			offset += m_FaceSize * m_FaceSize * bytesPerPixel;

			stbi_image_free(data);
		}

		size_t faceOffset = static_cast<size_t>(m_FaceSize) * m_FaceSize * 4;
		RotateFaceCounterClockwise(m_Data.Data + 2 * faceOffset);
		RotateFaceClockwise(m_Data.Data + 3 * faceOffset);

		Invalidate();
	}

	VulkanTextureCube::~VulkanTextureCube()
	{
		if (m_Data.Data)
		{
			delete[] m_Data.Data;
		}
	}

	void VulkanTextureCube::RegenerateMipMaps()
	{
		auto& commandBuffer = VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Graphics, true);
		vk::CommandBuffer vulkanCommandBuffer = (VkCommandBuffer)commandBuffer->GetHandle();

		// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
		vk::ImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		// We will transition on all mip levels
		subresourceRange.levelCount = m_MipLevelCount;
		// The 2D texture only has one layer
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 6;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_Image.Handle.get();
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;

		vulkanCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer,
											vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		GenerateMipMaps(vulkanCommandBuffer, m_Image, imageMemoryBarrier);

		VulkanContext::Get()->SubmitCommandBuffer(commandBuffer);
	}

	void VulkanTextureCube::Invalidate()
	{
		m_Image.Width = m_Image.Height = m_FaceSize;

		m_MipLevelCount = CalculateMaxMipMapCount(m_Image.Width, m_Image.Height);

		auto device = VulkanContext::GetDevice();
		auto deviceHandle = device->GetHandle();

		uint32 bytesPerPixel = GetBytesPerPixel(m_Format);

		// Create optimal tiled target image on the device
		vk::ImageCreateInfo imageCreateInfo{};
		imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.format = ConvertTextureFormatToVulkanFormat(m_Format);
		imageCreateInfo.mipLevels = m_MipLevelCount;
		imageCreateInfo.arrayLayers = 6;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		// Set initial layout of the image to undefined
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.extent = {m_FaceSize, m_FaceSize, 1};
		// TODO: Check which usages are necessary
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
								vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;
		m_Image.Handle = deviceHandle.createImageUnique(imageCreateInfo);

		VulkanBuffer stagingBuffer;
		uint32 size = m_Data.Size;

		m_Allocator.AllocateBuffer(stagingBuffer, size, vk::BufferUsageFlagBits::eTransferSrc,
								   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		m_Allocator.UpdateBuffer(stagingBuffer, m_Data.Data);

		vk::MemoryRequirements memoryRequirements = deviceHandle.getImageMemoryRequirements(m_Image.Handle.get());
		m_Allocator.Allocate(memoryRequirements, m_Image.DeviceMemory, vk::MemoryPropertyFlagBits::eDeviceLocal);
		deviceHandle.bindImageMemory(m_Image.Handle.get(), m_Image.DeviceMemory.get(), 0);

		auto& commandBuffer = VulkanContext::Get()->GetCommandBuffer(CommandBufferType::Graphics, true);
		vk::CommandBuffer copyCmd = (VkCommandBuffer)commandBuffer->GetHandle();

		// Image memory barriers for the texture image

		// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
		vk::ImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		// We will transition on all mip levels
		subresourceRange.levelCount = m_MipLevelCount;
		// The 2D texture only has one layer
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 6;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = m_Image.Handle.get();
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
		// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
		// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0,
								nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		vk::BufferImageCopy bufferCopyRegions[6];
		for (uint32 i = 0; i < 6; i++)
		{
			bufferCopyRegions[i].imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			bufferCopyRegions[i].imageSubresource.mipLevel = 0;
			bufferCopyRegions[i].imageSubresource.baseArrayLayer = i;
			bufferCopyRegions[i].imageSubresource.layerCount = 1;
			bufferCopyRegions[i].imageExtent.width = m_FaceSize;
			bufferCopyRegions[i].imageExtent.height = m_FaceSize;
			bufferCopyRegions[i].imageExtent.depth = 1;
			bufferCopyRegions[i].bufferOffset = static_cast<size_t>(i) * bytesPerPixel * m_FaceSize * m_FaceSize;
		}

		// Copy mip levels from staging buffer
		copyCmd.copyBufferToImage(stagingBuffer.Handle.get(), m_Image.Handle.get(), vk::ImageLayout::eTransferDstOptimal, 6,
								  bufferCopyRegions);

		GenerateMipMaps(copyCmd, m_Image, imageMemoryBarrier);

		// Store current layout for later reuse
		m_Layout = imageMemoryBarrier.newLayout;

		VulkanContext::Get()->SubmitCommandBuffer(commandBuffer);

		// Create a texture sampler
		// In Vulkan textures are accessed by samplers
		// This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
		// Note: Similar to the samplers available with OpenGL 3.3
		vk::SamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.minFilter = vk::Filter::eLinear;
		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerCreateInfo.addressModeU = ConvertTextureWrapToVulkanTextureWrap(m_Specification.Wrap);
		samplerCreateInfo.addressModeV = ConvertTextureWrapToVulkanTextureWrap(m_Specification.Wrap);
		samplerCreateInfo.addressModeW = ConvertTextureWrapToVulkanTextureWrap(m_Specification.Wrap);
		samplerCreateInfo.mipLodBias = 0.f;
		samplerCreateInfo.minLod = 0.f;
		samplerCreateInfo.maxLod = static_cast<float>(m_MipLevelCount);
		samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;

		// Enable anisotropic filtering
		// This feature is optional, so we must check if it's supported on the device
		if (device->GetPhysicalDevice()->GetSupportedFeatures().samplerAnisotropy)
		{
			// Use max. level of anisotropy for this example
			samplerCreateInfo.maxAnisotropy = device->GetPhysicalDevice()->GetProperties().limits.maxSamplerAnisotropy;
			samplerCreateInfo.anisotropyEnable = VK_TRUE;
		}
		else
		{
			// The device does not support anisotropic filtering
			samplerCreateInfo.maxAnisotropy = 1.0;
			samplerCreateInfo.anisotropyEnable = VK_FALSE;
		}
		m_Sampler = deviceHandle.createSamplerUnique(samplerCreateInfo);

		// Create image view
		// Textures are not directly accessed by the shaders and
		// are abstracted by image views containing additional
		// information and sub resource ranges
		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.viewType = vk::ImageViewType::eCube;
		imageViewCreateInfo.format = ConvertTextureFormatToVulkanFormat(m_Format);
		imageViewCreateInfo.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB,
										  vk::ComponentSwizzle::eA};
		// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
		// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 6;
		// The view will be based on the texture's image
		imageViewCreateInfo.image = m_Image.Handle.get();

		// Linear tiling usually won't support mip maps
		// Only set mip map count if optimal tiling is used
		for (uint32 i = 0; i < m_MipLevelCount; i++)
		{
			imageViewCreateInfo.subresourceRange.levelCount = m_MipLevelCount - i;
			imageViewCreateInfo.subresourceRange.baseMipLevel = i;
			m_Views.push_back(deviceHandle.createImageViewUnique(imageViewCreateInfo));
		}
	}

	void VulkanTextureCube::CreateDefaultTexture()
	{
		m_Format = TextureFormat::RGBA8;

		int width = 1, height = 1;
		auto* color = new glm::u8vec4[6];
		for (uint32 i = 0; i < 6; i++)
		{
			color[i] = {30, 30, 30, 255};
		}

		m_Data.Data = reinterpret_cast<stbi_uc*>(color);
		m_Data.Size = 6 * width * height * GetBytesPerPixel(m_Format);

		m_FaceSize = width;

		m_Image.Width = width;
		m_Image.Height = height;

		Invalidate();
	}

	void VulkanTextureCube::GetFace(const byte* sourceData, byte* destData, uint32 xOffset, uint32 yOffset)
	{
		uint32 bytesPerPixel = GetBytesPerPixel(m_Format);

		uint32 imageWidth = m_FaceSize * bytesPerPixel;
		for (uint32 y = 0; y < m_FaceSize; y++)
		{
			uint32 sourceY = y + yOffset;
			for (uint32 x = 0; x < m_FaceSize; x++)
			{
				uint32 sourceX = x + xOffset;

				uint32 baseSourceIndex = (sourceY * imageWidth + sourceX) * bytesPerPixel;
				uint32 baseDestinationIndex = (y * m_FaceSize + x) * bytesPerPixel;
				for (uint32 i = 0; i < bytesPerPixel; i++)
				{
					destData[baseDestinationIndex + i] = sourceData[baseSourceIndex + i];
				}
			}
		}
	}

	void VulkanTextureCube::RotateFaceClockwise(byte* data)
	{
		uint32 bytesPerPixel = GetBytesPerPixel(m_Format);

		auto* stagingData = new byte[static_cast<size_t>(m_FaceSize) * m_FaceSize * bytesPerPixel];

		for (uint32 y = 0; y < m_FaceSize; y++)
		{
			for (uint32 x = 0; x < m_FaceSize; x++)
			{
				uint32 sourceX = y;
				uint32 sourceY = m_FaceSize - x - 1;

				uint32 baseSourceIndex = (sourceY * m_FaceSize + sourceX) * bytesPerPixel;
				uint32 baseDestinationIndex = (y * m_FaceSize + x) * bytesPerPixel;
				for (uint32 i = 0; i < bytesPerPixel; i++)
				{
					stagingData[baseDestinationIndex + i] = data[baseSourceIndex + i];
				}
			}
		}

		memcpy(data, stagingData, static_cast<size_t>(m_FaceSize) * m_FaceSize * bytesPerPixel);

		delete[] stagingData;
	}

	void VulkanTextureCube::RotateFaceCounterClockwise(byte* data)
	{
		uint32 bytesPerPixel = GetBytesPerPixel(m_Format);

		auto* stagingData = new byte[static_cast<size_t>(m_FaceSize) * m_FaceSize * bytesPerPixel];

		for (uint32 y = 0; y < m_FaceSize; y++)
		{
			for (uint32 x = 0; x < m_FaceSize; x++)
			{
				uint32 sourceX = m_FaceSize - y - 1;
				uint32 sourceY = x;

				uint32 baseSourceIndex = (sourceY * m_FaceSize + sourceX) * bytesPerPixel;
				uint32 baseDestinationIndex = (y * m_FaceSize + x) * bytesPerPixel;
				for (uint32 i = 0; i < bytesPerPixel; i++)
				{
					stagingData[baseDestinationIndex + i] = data[baseSourceIndex + i];
				}
			}
		}

		memcpy(data, stagingData, static_cast<size_t>(m_FaceSize) * m_FaceSize * bytesPerPixel);

		delete[] stagingData;
	}

} // namespace Neon
