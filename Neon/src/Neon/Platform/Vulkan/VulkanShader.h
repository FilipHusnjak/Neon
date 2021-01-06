#pragma once

#include "Neon/Renderer/Shader.h"
#include "Vulkan.h"
#include "VulkanAllocator.h"

namespace Neon
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::vector<UniformBinding>& bindings);
		~VulkanShader() = default;

		void LoadShader(const std::string& path, ShaderType type) override;

		void SetUniformBuffer(uint32 binding, uint32 index, const void* data) override;

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
		std::vector<vk::UniqueShaderModule> m_ShaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;

		VulkanAllocator m_Allocator;

		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSetLayout m_DescriptorSetLayout;
		vk::UniqueDescriptorSet m_DescriptorSet;

		std::unordered_map<uint32, std::vector<VulkanBuffer>> m_UniformBuffers;
	};
} // namespace Neon
