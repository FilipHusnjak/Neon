#include "neopch.h"

#include "Pipeline.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Renderer.h"

namespace Neon
{
	SharedRef<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
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
				return SharedRef<VulkanPipeline>::Create(spec);
			}
		}
		NEO_CORE_ASSERT(false, "Renderer API not selected!");
		return nullptr;
	}
} // namespace Neon
