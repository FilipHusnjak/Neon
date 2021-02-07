#include "neopch.h"

#include "Pipeline.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Renderer.h"

namespace Neon
{
	Pipeline::Pipeline(const SharedRef<Shader>& shader)
		: m_Shader(shader)
	{
	}

	SharedRef<GraphicsPipeline> GraphicsPipeline::Create(const SharedRef<Shader>& shader, const GraphicsPipelineSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				NEO_CORE_ASSERT(false, "Renderer API not selected!");
				return nullptr;
			}
			case RendererAPI::API::Vulkan:
			{
				return SharedRef<VulkanGraphicsPipeline>::Create(shader, spec);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}

	GraphicsPipeline::GraphicsPipeline(const SharedRef<Shader>& shader, const GraphicsPipelineSpecification& specification)
		: Pipeline(shader)
		, m_Specification(specification)
	{
	}

	SharedRef<ComputePipeline> ComputePipeline::Create(const SharedRef<Shader>& shader, const ComputePipelineSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				NEO_CORE_ASSERT(false, "Renderer API not selected!");
				return nullptr;
			}
			case RendererAPI::API::Vulkan:
			{
				return SharedRef<VulkanComputePipeline>::Create(shader, spec);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}

	ComputePipeline::ComputePipeline(const SharedRef<Shader>& shader, const ComputePipelineSpecification& specification)
		: Pipeline(shader)
		, m_Specification(specification)
	{
	}

} // namespace Neon
