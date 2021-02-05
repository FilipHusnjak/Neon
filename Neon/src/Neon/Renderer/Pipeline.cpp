#include "neopch.h"

#include "Pipeline.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Renderer.h"

namespace Neon
{
	SharedRef<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineSpecification& spec)
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
				return SharedRef<VulkanGraphicsPipeline>::Create(spec);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}

	GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineSpecification& specification)
		: m_Specification(specification)
	{
	}

	SharedRef<ComputePipeline> ComputePipeline::Create(const ComputePipelineSpecification& spec)
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
				return SharedRef<VulkanComputePipeline>::Create(spec);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}

	ComputePipeline::ComputePipeline(const ComputePipelineSpecification& specification)
		: m_Specification(specification)
	{
	}

} // namespace Neon
