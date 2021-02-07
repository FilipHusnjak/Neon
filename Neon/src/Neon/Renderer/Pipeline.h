#pragma once

#include "RenderPass.h"
#include "Shader.h"
#include "VertexBuffer.h"

namespace Neon
{
	struct GraphicsPipelineSpecification
	{
		SharedRef<RenderPass> Pass;
	};

	struct ComputePipelineSpecification
	{
	};

	enum class PipelineBindPoint
	{
		Graphics,
		Compute
	};

	class Pipeline : public RefCounted
	{
	public:
		Pipeline(const SharedRef<Shader>& shader);
		virtual ~Pipeline() = default;

		virtual void* GetHandle() const = 0;
		virtual void* GetLayout() const = 0;

		virtual PipelineBindPoint GetBindPoint() const = 0;

		const SharedRef<Shader>& GetShader() const
		{
			return m_Shader;
		}

	protected:
		SharedRef<Shader> m_Shader;
	};

	class GraphicsPipeline : public Pipeline
	{
	public:
		static SharedRef<GraphicsPipeline> Create(const SharedRef<Shader>& shader, const GraphicsPipelineSpecification& spec);

	public:
		GraphicsPipeline(const SharedRef<Shader>& shader, const GraphicsPipelineSpecification& specification);

		GraphicsPipelineSpecification& GetSpecification()
		{
			return m_Specification;
		}
		const GraphicsPipelineSpecification& GetSpecification() const
		{
			return m_Specification;
		}

		PipelineBindPoint GetBindPoint() const override
		{
			return PipelineBindPoint::Graphics;
		}

	protected:
		GraphicsPipelineSpecification m_Specification;
	};

	class ComputePipeline : public Pipeline
	{
	public:
		static SharedRef<ComputePipeline> Create(const SharedRef<Shader>& shader, const ComputePipelineSpecification& spec);

	public:
		ComputePipeline(const SharedRef<Shader>& shader, const ComputePipelineSpecification& specification);

		ComputePipelineSpecification& GetSpecification()
		{
			return m_Specification;
		}
		const ComputePipelineSpecification& GetSpecification() const
		{
			return m_Specification;
		}

		PipelineBindPoint GetBindPoint() const override
		{
			return PipelineBindPoint::Compute;
		}

	protected:
		ComputePipelineSpecification m_Specification;
	};
} // namespace Neon
