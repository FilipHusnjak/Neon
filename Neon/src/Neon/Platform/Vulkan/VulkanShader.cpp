#include "neopch.h"

#include "Neon/Platform/Vulkan/VulkanContext.h"
#include "Neon/Platform/Vulkan/VulkanShader.h"
#include "Neon/Platform/Vulkan/VulkanTexture.h"
#include "Neon/Tools/FileTools.h"

#include <spirv_glsl.hpp>

#include <filesystem>

namespace Neon
{
	static vk::ShaderStageFlagBits ShaderTypeToVulkanShaderType(ShaderType shaderType)
	{
		switch (shaderType)
		{
			case ShaderType::Geometry:
				return vk::ShaderStageFlagBits::eGeometry;
			case ShaderType::Vertex:
				return vk::ShaderStageFlagBits::eVertex;
			case ShaderType::Fragment:
				return vk::ShaderStageFlagBits::eFragment;
			case ShaderType::Compute:
				return vk::ShaderStageFlagBits::eCompute;
			default:
				NEO_CORE_ASSERT(false, "Uknown shader type!");
				return vk::ShaderStageFlagBits();
		}
	}

	VulkanShader::VulkanShader(const ShaderSpecification& specification)
		: Shader(specification)
	{
		const auto& device = VulkanContext::GetDevice();
		m_Allocator = VulkanAllocator(device, "Shader");
		Reload();
	}

	void VulkanShader::Reload()
	{
		for (const auto& [shaderType, shaderPath] : m_Specification.ShaderPaths)
		{
			std::vector<char> shaderSource;
			bool fileRead = File::ReadFromFile(shaderPath, shaderSource, true);
			std::string strShader(shaderSource.begin(), shaderSource.end());
			NEO_CORE_ASSERT(fileRead, "Shader path does not exist!");
			m_ShaderSources[shaderType] = std::string(shaderSource.begin(), shaderSource.end());
		}

		for (const auto& [shaderType, shaderSource] : m_ShaderSources)
		{
			std::vector<uint32> shaderBinary;
			GetVulkanShaderBinary(shaderType, shaderBinary, false);
			CreateShader(shaderType, shaderBinary);
			Reflect(shaderType, shaderBinary);
		}
		CreateDescriptors();
	}

	void VulkanShader::SetUniformBuffer(const std::string& name, uint32 index, const void* data, uint32 size /*= 0*/)
	{
		NEO_CORE_ASSERT(m_UniformBuffers.find(name) != m_UniformBuffers.end(), "Unknown uniform buffer name!");
		NEO_CORE_ASSERT(index < m_UniformBuffers[name].Count, "Descriptor index out of range!");
		m_Allocator.UpdateBuffer(m_UniformBuffers[name].Buffers[index], data, size);
	}

	void VulkanShader::SetStorageBuffer(const std::string& name, const void* data, uint32 size /*= 0*/)
	{
		NEO_CORE_ASSERT(m_StorageBuffers.find(name) != m_StorageBuffers.end(), "Unknown storage buffer name!");
		m_Allocator.UpdateBuffer(m_StorageBuffers[name].BufferData, data, size);
	}

	void VulkanShader::SetPushConstant(const std::string& name, const void* data, uint32 size /*= 0*/)
	{
		NEO_CORE_ASSERT(m_PushConstants.find(name) != m_PushConstants.end(), "Unknown push constant name!");
		NEO_CORE_ASSERT(size <= m_PushConstants[name].Size);
		size = size == 0 ? m_PushConstants[name].Size : size;
		memcpy(m_PushConstants[name].Data.get(), data, size);
	}

	void VulkanShader::SetTexture2D(const std::string& name, uint32 index, const SharedRef<Texture2D>& texture, uint32 mipLevel)
	{
		NEO_CORE_ASSERT(index < m_ImageSamplers[name].Count);

		const auto vulkanTexture = texture.As<VulkanTexture2D>();
		auto& imageInfo = vulkanTexture->GetTextureDescription(mipLevel);

		vk::WriteDescriptorSet descWrite{
			m_DescriptorSet.get(), m_NameBindingMap[name], index, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo};
		m_PendingWrites.emplace_back(nullptr, imageInfo, descWrite);

		RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(m_ImageSamplers[name].Textures[index]));
		m_ImageSamplers[name].Textures[index] = texture;
	}

	void VulkanShader::SetTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& texture, uint32 mipLevel)
	{
		NEO_CORE_ASSERT(index < m_ImageSamplers[name].Count);

		const auto vulkanTexture = texture.As<VulkanTextureCube>();
		auto& imageInfo = vulkanTexture->GetTextureDescription(mipLevel);

		vk::WriteDescriptorSet descWrite{
			m_DescriptorSet.get(), m_NameBindingMap[name], index, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo};
		m_PendingWrites.emplace_back(nullptr, imageInfo, descWrite);

		RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(m_ImageSamplers[name].Textures[index]));
		m_ImageSamplers[name].Textures[index] = texture;
	}

	void VulkanShader::SetStorageTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& texture,
											 uint32 mipLevel)
	{
		NEO_CORE_ASSERT(index < m_StorageImages[name].Count);

		const auto vulkanTexture = texture.As<VulkanTextureCube>();
		auto& imageInfo = vulkanTexture->GetTextureDescription(mipLevel);

		vk::WriteDescriptorSet descWrite{
			m_DescriptorSet.get(), m_NameBindingMap[name], index, 1, vk::DescriptorType::eStorageImage, &imageInfo};
		m_PendingWrites.emplace_back(nullptr, imageInfo, descWrite);

		RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(m_StorageImages[name].Textures[index]));
		m_StorageImages[name].Textures[index] = texture;
	}

	SharedRef<Texture2D> VulkanShader::GetTexture2D(const std::string& name, uint32 index) const
	{
		NEO_CORE_ASSERT(m_ImageSamplers.find(name) != m_ImageSamplers.end());
		NEO_CORE_ASSERT(index >= 0 && index < m_ImageSamplers.at(name).Textures.size());
		return m_ImageSamplers.at(name).Textures[index].As<Texture2D>();
	}

	SharedRef<TextureCube> VulkanShader::GetTextureCube(const std::string& name, uint32 index) const
	{
		NEO_CORE_ASSERT(m_ImageSamplers.find(name) != m_ImageSamplers.end());
		NEO_CORE_ASSERT(index >= 0 && index < m_ImageSamplers.at(name).Textures.size());
		return m_ImageSamplers.at(name).Textures[index].As<TextureCube>();
	}

	void VulkanShader::PrepareDescriptorSet()
	{
		if (!m_PendingWrites.empty())
		{
			vk::Device device = VulkanContext::GetDevice()->GetHandle();

			std::vector<vk::WriteDescriptorSet> writes;

			for (auto& [bufferInfo, imageInfo, descWrite] : m_PendingWrites)
			{
				descWrite.pBufferInfo = &bufferInfo;
				descWrite.pImageInfo = &imageInfo;
				writes.push_back(descWrite);
			}

			device.updateDescriptorSets(writes, {});

			m_PendingWrites.clear();
		}
	}

	void VulkanShader::GetVulkanShaderBinary(ShaderType shaderType, std::vector<uint32>& outShaderBinary, bool forceCompile)
	{
		std::string shaderPath = m_Specification.ShaderPaths.at(shaderType);
		std::filesystem::path p = shaderPath;

		std::string folderPath = (p.parent_path() / "cached\\").string();
		wchar_t wtext[40];
		mbstowcs(wtext, folderPath.c_str(), folderPath.length());
		wtext[folderPath.length()] = '\0';
		CreateDirectory(wtext, nullptr);

		auto cachePath = p.parent_path() / "cached" / (p.filename().string() + ".cached_vulkan");
		std::string cachedFilePath = cachePath.string();

		File::ReadFromFile(cachedFilePath, outShaderBinary, true);

		if (forceCompile || outShaderBinary.empty())
		{
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

			const bool optimize = false;
			if (optimize)
			{
				options.SetOptimizationLevel(shaderc_optimization_level_performance);
			}

			auto& shaderSource = m_ShaderSources.at(shaderType);
			shaderc::SpvCompilationResult module =
				compiler.CompileGlslToSpv(shaderSource, ShaderTypeToShadercShaderType(shaderType), shaderPath.c_str(), options);

			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				NEO_CORE_ERROR(module.GetErrorMessage());
				NEO_CORE_ASSERT(false);
			}

			outShaderBinary = std::vector<uint32>(module.cbegin(), module.cend());

			File::WriteToFile(cachedFilePath, outShaderBinary, true);
		}
	}

	void VulkanShader::CreateShader(ShaderType shaderType, const std::vector<uint32>& shaderBinary)
	{
		const vk::Device device = VulkanContext::GetDevice()->GetHandle();
		vk::ShaderModuleCreateInfo createInfo{{}, shaderBinary.size() * sizeof(uint32), shaderBinary.data()};
		m_ShaderModules.push_back(device.createShaderModuleUnique(createInfo));

		m_ShaderStages.push_back({{}, ShaderTypeToVulkanShaderType(shaderType), m_ShaderModules.back().get(), "main"});
	}

	void VulkanShader::Reflect(ShaderType shaderType, const std::vector<uint32>& shaderBinary)
	{
		vk::Device device = VulkanContext::GetDevice()->GetHandle();

		NEO_CORE_TRACE("===========================");
		NEO_CORE_TRACE(" Vulkan Shader Reflection");
		NEO_CORE_TRACE(" {0}", m_Specification.ShaderPaths.at(shaderType));
		NEO_CORE_TRACE("===========================");

		spirv_cross::Compiler compiler(shaderBinary);
		auto resources = compiler.get_shader_resources();

		NEO_CORE_TRACE("Uniform Buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& name = resource.name;
			auto& bufferType = compiler.get_type(resource.type_id);
			uint32 bindingPoint = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 count = std::max(1u, bufferType.array[0]);
			if (m_Specification.ShaderVariableCounts.find(name) != m_Specification.ShaderVariableCounts.end())
			{
				count = m_Specification.ShaderVariableCounts[name];
			}

			auto size = static_cast<uint32>(compiler.get_declared_struct_size(bufferType));
			auto memberCount = static_cast<uint32>(bufferType.member_types.size());

			m_NameBindingMap[name] = bindingPoint;
			UniformBuffer& buffer = m_UniformBuffers[name];
			buffer.Name = name;
			buffer.BindingPoint = bindingPoint;
			buffer.Count = count;
			buffer.Size = size;
			buffer.ShaderStage |= ShaderTypeToVulkanShaderType(shaderType);

			NEO_CORE_TRACE("  Name: {0}", name);
			NEO_CORE_TRACE("  Count: {0}", count);
			NEO_CORE_TRACE("  Member Count: {0}", memberCount);
			NEO_CORE_TRACE("  Binding Point: {0}", bindingPoint);
			NEO_CORE_TRACE("  Size: {0}", size);
			NEO_CORE_TRACE("-------------------");
		}

		NEO_CORE_TRACE("Storage Buffers:");
		for (const auto& resource : resources.storage_buffers)
		{
			const auto& name = resource.name;
			auto& bufferType = compiler.get_type(resource.type_id);
			uint32 bindingPoint = compiler.get_decoration(resource.id, spv::DecorationBinding);

			auto size = static_cast<uint32>(compiler.get_declared_struct_size(bufferType));
			auto memberCount = static_cast<uint32>(bufferType.member_types.size());
			if (size == 0)
			{
				for (uint32 i = 0; i < memberCount; i++)
				{
					size += compiler.type_struct_member_array_stride(bufferType, i);
				}
			}

			m_NameBindingMap[name] = bindingPoint;
			StorageBuffer& buffer = m_StorageBuffers[name];
			buffer.Name = name;
			buffer.BindingPoint = bindingPoint;
			buffer.Size = size * m_Specification.ShaderVariableCounts[name];
			buffer.ShaderStage |= ShaderTypeToVulkanShaderType(shaderType);

			NEO_CORE_TRACE("  Name: {0}", name);
			NEO_CORE_TRACE("  Member Count: {0}", memberCount);
			NEO_CORE_TRACE("  Binding Point: {0}", bindingPoint);
			NEO_CORE_TRACE("  Size: {0}", size);
			NEO_CORE_TRACE("-------------------");
		}

		NEO_CORE_TRACE("Image Samplers:");
		for (const auto& resource : resources.sampled_images)
		{
			const auto& name = resource.name;
			auto& imageSamplerType = compiler.get_type(resource.base_type_id);
			uint32 bindingPoint = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 count = std::max(1u, imageSamplerType.array[0]);
			if (m_Specification.ShaderVariableCounts.find(name) != m_Specification.ShaderVariableCounts.end())
			{
				count = m_Specification.ShaderVariableCounts[name];
			}
			uint32 dimension = imageSamplerType.image.dim;

			m_NameBindingMap[name] = bindingPoint;
			auto& imageSampler = m_ImageSamplers[name];
			imageSampler.BindingPoint = bindingPoint;
			imageSampler.Name = name;
			imageSampler.Count = count;
			imageSampler.ShaderStage |= ShaderTypeToVulkanShaderType(shaderType);
			imageSampler.Textures.resize(count);

			NEO_CORE_TRACE("  Name: {0}", name);
			NEO_CORE_TRACE("  Count: {0}", count);
			NEO_CORE_TRACE("  Binding Point: {0}", bindingPoint);
			NEO_CORE_TRACE("-------------------");
		}

		NEO_CORE_TRACE("Storage Images:");
		for (const auto& resource : resources.storage_images)
		{
			const auto& name = resource.name;
			auto& storageImageType = compiler.get_type(resource.base_type_id);
			uint32 bindingPoint = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32 count = std::max(1u, storageImageType.array[0]);
			if (m_Specification.ShaderVariableCounts.find(name) != m_Specification.ShaderVariableCounts.end())
			{
				count = m_Specification.ShaderVariableCounts[name];
			}
			uint32 dimension = storageImageType.image.dim;

			m_NameBindingMap[name] = bindingPoint;
			auto& storageImage = m_StorageImages[name];
			storageImage.BindingPoint = bindingPoint;
			storageImage.Name = name;
			storageImage.Count = count;
			storageImage.ShaderStage |= ShaderTypeToVulkanShaderType(shaderType);
			storageImage.Textures.resize(count);

			NEO_CORE_TRACE("  Name: {0}", name);
			NEO_CORE_TRACE("  Count: {0}", count);
			NEO_CORE_TRACE("  Binding Point: {0}", bindingPoint);
			NEO_CORE_TRACE("-------------------");
		}

		NEO_CORE_TRACE("Push Constants:");
		for (const auto& resource : resources.push_constant_buffers)
		{
			const auto& name = resource.name;
			auto& pushConstantType = compiler.get_type(resource.base_type_id);

			auto size = static_cast<uint32>(compiler.get_declared_struct_size(pushConstantType));
			m_PushConstants[name].Name = name;
			m_PushConstants[name].Size = size;
			m_PushConstants[name].ShaderStage = ShaderTypeToVulkanShaderType(shaderType);
			m_PushConstants[name].Data = std::unique_ptr<byte>(new byte[size]);

			NEO_CORE_TRACE("  Name: {0}", name);
			NEO_CORE_TRACE("  Size: {0}", size);
			NEO_CORE_TRACE("-------------------");
		}

		NEO_CORE_TRACE("===========================");
	}

	void VulkanShader::CreateDescriptors()
	{
		vk::Device device = VulkanContext::GetDevice()->GetHandle();

		// We need to tell the API the number of max. requested descriptors per type
		std::vector<vk::DescriptorPoolSize> poolSizes;
		if (!m_UniformBuffers.empty())
		{
			vk::DescriptorPoolSize& typeCount = poolSizes.emplace_back();
			typeCount.type = vk::DescriptorType::eUniformBuffer;
			for (const auto& [binding, uniformBuffer] : m_UniformBuffers)
			{
				typeCount.descriptorCount += uniformBuffer.Count;
			}
		}
		if (!m_StorageBuffers.empty())
		{
			vk::DescriptorPoolSize& typeCount = poolSizes.emplace_back();
			typeCount.type = vk::DescriptorType::eStorageBuffer;
			typeCount.descriptorCount = static_cast<uint32>(m_StorageBuffers.size());
		}
		if (!m_ImageSamplers.empty())
		{
			vk::DescriptorPoolSize& typeCount = poolSizes.emplace_back();
			typeCount.type = vk::DescriptorType::eCombinedImageSampler;
			for (const auto& [binding, imageSampler] : m_ImageSamplers)
			{
				typeCount.descriptorCount += imageSampler.Count;
			}
		}
		if (!m_StorageImages.empty())
		{
			vk::DescriptorPoolSize& typeCount = poolSizes.emplace_back();
			typeCount.type = vk::DescriptorType::eStorageImage;
			for (const auto& [binding, storageImage] : m_StorageImages)
			{
				typeCount.descriptorCount += storageImage.Count;
			}
		}

		// TODO: Move this to the centralized renderer
		// Create the global descriptor pool
		// All descriptors used in this example are allocated from this pool
		vk::DescriptorPoolCreateInfo descPoolCreateInfo = {};
		descPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		descPoolCreateInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
		descPoolCreateInfo.pPoolSizes = poolSizes.data();
		descPoolCreateInfo.maxSets = 1;

		m_DescriptorPool = device.createDescriptorPoolUnique(descPoolCreateInfo);

		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
		for (auto& [name, uniformBuffer] : m_UniformBuffers)
		{
			auto& layoutBinding = layoutBindings.emplace_back();
			layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
			layoutBinding.descriptorCount = uniformBuffer.Count;
			layoutBinding.stageFlags = uniformBuffer.ShaderStage;
			layoutBinding.binding = m_NameBindingMap[name];

			uniformBuffer.Buffers.resize(uniformBuffer.Count);
			for (uint32 i = 0; i < uniformBuffer.Count; i++)
			{
				m_Allocator.AllocateBuffer(uniformBuffer.Buffers[i], uniformBuffer.Size, vk::BufferUsageFlagBits::eUniformBuffer,
										   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			}
		}
		for (auto& [name, storageBuffer] : m_StorageBuffers)
		{
			auto& layoutBinding = layoutBindings.emplace_back();
			layoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = storageBuffer.ShaderStage;
			layoutBinding.binding = m_NameBindingMap[name];

			m_Allocator.AllocateBuffer(storageBuffer.BufferData, storageBuffer.Size, vk::BufferUsageFlagBits::eStorageBuffer,
									   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		}
		for (auto& [name, imageSampler] : m_ImageSamplers)
		{
			auto& layoutBinding = layoutBindings.emplace_back();
			layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			layoutBinding.descriptorCount = imageSampler.Count;
			layoutBinding.stageFlags = imageSampler.ShaderStage;
			layoutBinding.binding = m_NameBindingMap[name];
		}
		for (auto& [name, storageImage] : m_StorageImages)
		{
			auto& layoutBinding = layoutBindings.emplace_back();
			layoutBinding.descriptorType = vk::DescriptorType::eStorageImage;
			layoutBinding.descriptorCount = storageImage.Count;
			layoutBinding.stageFlags = storageImage.ShaderStage;
			layoutBinding.binding = m_NameBindingMap[name];
		}

		vk::DescriptorSetLayoutCreateInfo descriptorLayout = {};
		descriptorLayout.bindingCount = static_cast<uint32>(layoutBindings.size());
		descriptorLayout.pBindings = layoutBindings.data();
		m_DescriptorSetLayout = device.createDescriptorSetLayoutUnique(descriptorLayout);

		vk::DescriptorSetAllocateInfo allocInfo(m_DescriptorPool.get(), 1, &m_DescriptorSetLayout.get());
		m_DescriptorSet = std::move(device.allocateDescriptorSetsUnique(allocInfo)[0]);

		for (const auto& [name, uniformBuffer] : m_UniformBuffers)
		{
			for (uint32 i = 0; i < uniformBuffer.Count; i++)
			{
				vk::DescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = m_UniformBuffers[name].Buffers[i].Handle.get();
				bufferInfo.offset = 0;
				bufferInfo.range = m_UniformBuffers[name].Buffers[i].Size;

				vk::WriteDescriptorSet descWrite = {
					m_DescriptorSet.get(), m_NameBindingMap[name], i, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo};
				m_PendingWrites.emplace_back(bufferInfo, nullptr, descWrite);
			}
		}

		for (const auto& [name, storageBuffer] : m_StorageBuffers)
		{
			vk::DescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = m_StorageBuffers[name].BufferData.Handle.get();
			bufferInfo.offset = 0;
			bufferInfo.range = m_StorageBuffers[name].BufferData.Size;

			vk::WriteDescriptorSet descWrite = {
				m_DescriptorSet.get(), m_NameBindingMap[name], 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &bufferInfo};
			m_PendingWrites.emplace_back(bufferInfo, nullptr, descWrite);
		}
	}

} // namespace Neon
