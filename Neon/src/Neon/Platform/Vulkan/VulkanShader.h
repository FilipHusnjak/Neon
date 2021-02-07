#pragma once

#include "Neon/Renderer/Shader.h"
#include "Vulkan.h"
#include "VulkanAllocator.h"

namespace Neon
{
	class VulkanShader : public Shader
	{
	public:
		struct UniformBuffer
		{
			std::string Name;
			uint32 BindingPoint = 0;
			uint32 Count = 0;
			uint32 Size = 0;
			std::vector<VulkanBuffer> Buffers;
			vk::ShaderStageFlags ShaderStage;
		};

		struct StorageBuffer
		{
			std::string Name;
			uint32 BindingPoint = 0;
			uint32 Size = 0;
			VulkanBuffer BufferData;
			vk::ShaderStageFlags ShaderStage;
		};

		struct ImageSampler
		{
			std::string Name;
			uint32 BindingPoint = 0;
			uint32 Count = 0;
			vk::ShaderStageFlags ShaderStage;
			SharedRef<Texture> Texture;
		};

		struct StorageImage
		{
			std::string Name;
			uint32 BindingPoint = 0;
			uint32 Count = 0;
			vk::ShaderStageFlags ShaderStage;
			SharedRef<Texture> Texture;
		};

		struct PushConstant
		{
			std::string Name;
			uint32 Size;
			vk::ShaderStageFlags ShaderStage;
			UniqueRef<byte> Data;
		};

		VulkanShader(const ShaderSpecification& shaderSpecification);
		~VulkanShader() = default;

		void Reload() override;

		void SetUniformBuffer(const std::string& name, uint32 index, const void* data, uint32 size = 0) override;
		void SetStorageBuffer(const std::string& name, const void* data, uint32 size = 0) override;
		void SetPushConstant(const std::string& name, const void* data, uint32 size = 0) override;
		void SetTexture2D(const std::string& name, uint32 index, const SharedRef<Texture2D>& texture) override;
		void SetTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& texture) override;
		void SetStorageTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& texture) override;

		vk::DescriptorSetLayout GetDescriptorSetLayout() const
		{
			return m_DescriptorSetLayout.get();
		}
		const std::vector<vk::PipelineShaderStageCreateInfo>& GetShaderStages() const
		{
			return m_ShaderStages;
		}

	private:
		void GetVulkanShaderBinary(ShaderType shaderType, std::vector<uint32>& outShaderBinary, bool forceCompile);
		void CreateShader(ShaderType shaderType, const std::vector<uint32>& shaderBinary);
		void Reflect(ShaderType shaderType, const std::vector<uint32>& shaderBinary);

		void CreateDescriptors();

	private:
		std::vector<vk::UniqueShaderModule> m_ShaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
		std::unordered_map<ShaderType, std::string> m_ShaderSources;

		VulkanAllocator m_Allocator;

		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSetLayout m_DescriptorSetLayout;
		vk::UniqueDescriptorSet m_DescriptorSet;

		std::unordered_map<std::string, UniformBuffer> m_UniformBuffers;
		std::unordered_map<std::string, StorageBuffer> m_StorageBuffers;
		std::unordered_map<std::string, ImageSampler> m_ImageSamplers;
		std::unordered_map<std::string, StorageImage> m_StorageImages;
		std::unordered_map<std::string, PushConstant> m_PushConstants;

		std::unordered_map<std::string, uint32> m_NameBindingMap;

		friend class VulkanCommandBuffer;
		friend class VulkanGraphicsPipeline;
		friend class VulkanComputePipeline;
	};
} // namespace Neon
