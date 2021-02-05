#pragma once

#include "RenderPass.h"
#include "Shader.h"
#include "VertexBuffer.h"

namespace Neon
{
	struct GraphicsPipelineSpecification
	{
		SharedRef<Shader> Shader;
		SharedRef<RenderPass> Pass;
	};

	struct ComputePipelineSpecification
	{
		SharedRef<Shader> Shader;
	};

	class Pipeline : public RefCounted
	{
	public:
		virtual ~Pipeline() = default;

		virtual void* GetHandle() const = 0;
	};

	class GraphicsPipeline : public Pipeline
	{
	public:
		static SharedRef<GraphicsPipeline> Create(const GraphicsPipelineSpecification& spec);

	public:
		GraphicsPipeline(const GraphicsPipelineSpecification& specification);

		GraphicsPipelineSpecification& GetSpecification()
		{
			return m_Specification;
		}
		const GraphicsPipelineSpecification& GetSpecification() const
		{
			return m_Specification;
		}

	protected:
		GraphicsPipelineSpecification m_Specification;
	};

	class ComputePipeline : public Pipeline
	{
	public:
		static SharedRef<ComputePipeline> Create(const ComputePipelineSpecification& spec);

	public:
		ComputePipeline(const ComputePipelineSpecification& specification);

		ComputePipelineSpecification& GetSpecification()
		{
			return m_Specification;
		}
		const ComputePipelineSpecification& GetSpecification() const
		{
			return m_Specification;
		}

	protected:
		ComputePipelineSpecification m_Specification;
	};
} // namespace Neon
