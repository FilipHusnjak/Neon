#pragma once

#include "Renderer/Pipeline.h"
#include "Vulkan.h"

namespace Neon
{
	static vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Int:
				return vk::Format::eR32Sint;
			case ShaderDataType::UInt:
				return vk::Format::eR32Uint;
			case ShaderDataType::Float:
				return vk::Format::eR32Sfloat;
			case ShaderDataType::Float2:
				return vk::Format::eR32G32Sfloat;
			case ShaderDataType::Float3:
				return vk::Format::eR32G32B32Sfloat;
			case ShaderDataType::Float4:
				return vk::Format::eR32G32B32A32Sfloat;
		}
		NEO_CORE_ASSERT(false, "Uknown shader data type!");
		return vk::Format::eUndefined;
	}

	class VulkanGraphicsPipeline : public GraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specification);
		~VulkanGraphicsPipeline() = default;

		vk::PipelineLayout GetLayout() const
		{
			return m_PipelineLayout.get();
		}

		void* GetHandle() const override
		{
			return m_Handle.get();
		}

	private:
		vk::UniquePipelineLayout m_PipelineLayout;
		vk::UniquePipeline m_Handle;
	};

	class VulkanComputePipeline : public ComputePipeline
	{
	public:
		VulkanComputePipeline(const ComputePipelineSpecification& specification);
		~VulkanComputePipeline() = default;

		vk::PipelineLayout GetLayout() const
		{
			return m_PipelineLayout.get();
		}

		void* GetHandle() const override
		{
			return m_Handle.get();
		}

	private:
		vk::UniquePipelineLayout m_PipelineLayout;
		vk::UniquePipeline m_Handle;
	};

} // namespace Neon
