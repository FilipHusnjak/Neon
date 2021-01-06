#include "neopch.h"

#include "Tools/FileTools.h"
#include "VulkanContext.h"
#include "VulkanShader.h"

#include <shaderc/shaderc.hpp>
#include <spirv_glsl.hpp>

namespace Neon
{
	VulkanShader::VulkanShader(const std::vector<UniformBinding>& bindings)
	{
		const auto& device = VulkanContext::GetDevice();

		m_Allocator = VulkanAllocator(device, "Shader");

		if (bindings.empty())
		{
			return;
		}

		std::vector<vk::DescriptorPoolSize> sizes;
		std::vector<vk::DescriptorSetLayoutBinding> descBindings;
		for (const auto& binding : bindings)
		{
			vk::DescriptorType descType = vk::DescriptorType::eUniformBuffer;
			switch (binding.Type)
			{
				case UniformType::UniformBuffer:
				{
					descType = vk::DescriptorType::eUniformBuffer;
					m_UniformBuffers[binding.Binding].resize(binding.Count);
					for (auto& uniformBuffer : m_UniformBuffers[binding.Binding])
					{
						m_Allocator.AllocateBuffer(uniformBuffer, binding.Size, vk::BufferUsageFlagBits::eUniformBuffer,
												   vk::MemoryPropertyFlagBits::eHostVisible |
													   vk::MemoryPropertyFlagBits::eHostCoherent);
					}
				}
				break;
				default:
				{
					NEO_CORE_ASSERT(false, "Uknown binding type!");
				}
				break;
			}

			vk::ShaderStageFlags stageFlags;
			if (binding.ShaderStageFlags & ShaderStageFlag::Vertex)
			{
				stageFlags |= vk::ShaderStageFlagBits::eVertex;
			}
			if (binding.ShaderStageFlags & ShaderStageFlag::Fragment)
			{
				stageFlags |= vk::ShaderStageFlagBits::eFragment;
			}

			vk::DescriptorSetLayoutBinding vulkanBinding = {binding.Binding, descType, binding.Count, stageFlags};
			descBindings.push_back(vulkanBinding);

			sizes.emplace_back(descType, binding.Count);
		}

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		poolInfo.poolSizeCount = static_cast<uint32>(sizes.size());
		poolInfo.pPoolSizes = sizes.data();
		poolInfo.maxSets = 1;
		m_DescriptorPool = device->GetHandle().createDescriptorPoolUnique(poolInfo);

		vk::DescriptorSetLayoutCreateInfo layoutInfo({}, static_cast<uint32>(descBindings.size()), descBindings.data());
		m_DescriptorSetLayout = device->GetHandle().createDescriptorSetLayoutUnique(layoutInfo);

		vk::DescriptorSetAllocateInfo allocInfo(m_DescriptorPool.get(), 1, &m_DescriptorSetLayout.get());
		m_DescriptorSet = std::move(device->GetHandle().allocateDescriptorSetsUnique(allocInfo)[0]);

		for (const auto& descBinding : descBindings)
		{
			switch (descBinding.descriptorType)
			{
				case vk::DescriptorType::eUniformBuffer:
				{
					std::vector<vk::DescriptorBufferInfo> bufferInfos(descBinding.descriptorCount);
					for (uint32 i = 0; i < bufferInfos.size(); i++)
					{
						bufferInfos[i].buffer = m_UniformBuffers[descBinding.binding][i].Handle.get();
						bufferInfos[i].offset = 0;
						bufferInfos[i].range = m_UniformBuffers[descBinding.binding][i].Size;
					}
					vk::WriteDescriptorSet descWrite = {m_DescriptorSet.get(),		 descBinding.binding,		 0,
														descBinding.descriptorCount, descBinding.descriptorType, nullptr,
														bufferInfos.data()};
					device->GetHandle().updateDescriptorSets({descWrite}, nullptr);
				}
				break;
				default:
				{
					NEO_CORE_ASSERT(false, "Uknown binding type!");
				}
				break;
			}
		}
	}

	void VulkanShader::LoadShader(const std::string& path, ShaderType type)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		const vk::Device device = VulkanContext::GetDevice()->GetHandle();
		std::vector<char> code = ReadFile(path);
		vk::ShaderModuleCreateInfo createInfo{{}, code.size(), reinterpret_cast<const uint32*>(code.data())};
		m_ShaderModules.push_back(device.createShaderModuleUnique(createInfo));

		vk::ShaderStageFlagBits stage;
		switch (type)
		{
			case ShaderType::Vertex:
				stage = vk::ShaderStageFlagBits::eVertex;
				break;
			case ShaderType::Fragment:
				stage = vk::ShaderStageFlagBits::eFragment;
				break;
			default:
				NEO_CORE_ASSERT(false, "Invalid shader type");
				break;
		}
		m_ShaderStages.push_back({{}, stage, m_ShaderModules.back().get(), "main"});
	}

	void VulkanShader::SetUniformBuffer(uint32 binding, uint32 index, const void* data)
	{
		NEO_CORE_ASSERT(m_UniformBuffers.find(binding) != m_UniformBuffers.end(), "Uniform binding is invalid!");
		NEO_CORE_ASSERT(index < m_UniformBuffers[binding].size(), "Descriptor index out of range!");
		m_Allocator.UpdateBuffer(m_UniformBuffers[binding][index], data);
	}
} // namespace Neon
