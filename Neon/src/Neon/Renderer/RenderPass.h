#pragma once

#include "Neon/Renderer/Framebuffer.h"

namespace Neon
{
	enum class FramebufferFormat
	{
		None,
		RGBA8,
		RGBA16F
	};

	struct RenderPassSpecification
	{
		bool HasColor = true;
		bool HasDepth = true;
		uint32 Samples = 1;
		FramebufferFormat ColorFormat = FramebufferFormat::RGBA8;
		SharedRef<Framebuffer> TargetFramebuffer;
	};

	class RenderPass : public RefCounted
	{
	public:
		RenderPass(const RenderPassSpecification& spec);
		virtual ~RenderPass() = default;

		void SetTargetFramebuffer(const SharedRef<Framebuffer>& targetFramebuffer)
		{
			m_Specification.TargetFramebuffer = targetFramebuffer;
		}

		RenderPassSpecification& GetSpecification()
		{
			return m_Specification;
		}
		const RenderPassSpecification& GetSpecification() const
		{
			return m_Specification;
		}

		virtual void* GetHandle() const = 0;

		static SharedRef<RenderPass> Create(const RenderPassSpecification& spec);

	protected:
		RenderPassSpecification m_Specification;
	};
} // namespace Neon
