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
			vk::DescriptorBufferInfo Descriptor;
			vk::ShaderStageFlagBits ShaderStage = vk::ShaderStageFlagBits::eAll;
		};

		struct ImageSampler
		{
			std::string Name;
			uint32 BindingPoint = 0;
			uint32 Count = 0;
			vk::ShaderStageFlagBits ShaderStage = vk::ShaderStageFlagBits::eAll;
		};

		VulkanShader(const ShaderSpecification& shaderSpecification,
					 const std::unordered_map<ShaderType, std::string>& shaderPaths);
		~VulkanShader() = default;

		void Reload() override;

		void SetUniformBuffer(uint32 binding, uint32 index, const void* data) override;
		void SetTexture(uint32 binding, uint32 index, const SharedRef<Texture2D>& texture) override;

		vk::DescriptorSet GetDescriptorSet() const
		{
			return m_DescriptorSet.get();
		}
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
		ShaderSpecification m_Specification;

		std::vector<vk::UniqueShaderModule> m_ShaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
		std::unordered_map<ShaderType, std::string> m_ShaderSources;
		std::unordered_map<ShaderType, std::string> m_ShaderPaths;

		VulkanAllocator m_Allocator;

		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSetLayout m_DescriptorSetLayout;
		vk::UniqueDescriptorSet m_DescriptorSet;

		std::unordered_map<uint32, UniformBuffer> m_UniformBuffers;
		std::unordered_map<uint32, ImageSampler> m_ImageSamplers;
	};
} // namespace Neon
