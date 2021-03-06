#pragma once

#include "Renderer/Pipeline.h"
#include "Vulkan.h"

namespace Neon
{
	static vk::Format ConvertNeonShaderDataTypeToVulkanDataType(ShaderDataType type)
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

	static vk::PipelineBindPoint NeonToVulkanPipelineBindPoint(PipelineBindPoint bindPoint)
	{
		switch (bindPoint)
		{
			case PipelineBindPoint::Graphics:
				return vk::PipelineBindPoint::eGraphics;
			case PipelineBindPoint::Compute:
				return vk::PipelineBindPoint::eCompute;
		}
		NEO_CORE_ASSERT(false, "Uknown pipeline bind point!");
		return vk::PipelineBindPoint::eGraphics;
	}

	static vk::PolygonMode ConvertNeonPolygonModeToVulkanPolygonMode(PolygonMode polygonMode)
	{
		switch (polygonMode)
		{
			case PolygonMode::Fill:
				return vk::PolygonMode::eFill;
			case PolygonMode::Line:
				return vk::PolygonMode::eLine;
		}
		NEO_CORE_ASSERT(false, "Uknown polygon mode!");
		return vk::PolygonMode::eFill;
	}

	class VulkanGraphicsPipeline : public GraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline(const SharedRef<Shader>& shader, const GraphicsPipelineSpecification& specification);
		~VulkanGraphicsPipeline() = default;

		void* GetLayout() const override
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
		VulkanComputePipeline(const SharedRef<Shader>& shader, const ComputePipelineSpecification& specification);
		~VulkanComputePipeline() = default;

		void* GetLayout() const override
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
