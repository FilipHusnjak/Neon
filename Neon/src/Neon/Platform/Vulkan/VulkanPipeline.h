#pragma once

#include "Renderer/Pipeline.h"
#include "Vulkan.h"

namespace Neon
{
	static vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType type)
	{
		switch (type)
		{
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

	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineSpecification& specification);
		~VulkanPipeline() = default;

		vk::PipelineLayout GetLayout() const
		{
			return m_PipelineLayout.get();
		}

		PipelineSpecification& GetSpecification() override
		{
			return m_Specification;
		}
		const PipelineSpecification& GetSpecification() const override
		{
			return m_Specification;
		}

		vk::Pipeline GetHandle() const
		{
			return m_Handle.get();
		}

	private:
		PipelineSpecification m_Specification;

		vk::UniquePipelineLayout m_PipelineLayout;
		vk::UniquePipeline m_Handle;
	};
} // namespace Neon
