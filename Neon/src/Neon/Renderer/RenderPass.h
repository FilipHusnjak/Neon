#pragma once

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
	};

	class RenderPass : public RefCounted
	{
	public:
		RenderPass(const RenderPassSpecification& spec);
		virtual ~RenderPass() = default;

		RenderPassSpecification& GetSpecification()
		{
			return m_Specification;
		}
		const RenderPassSpecification& GetSpecification() const
		{
			return m_Specification;
		}

		static SharedRef<RenderPass> Create(const RenderPassSpecification& spec);

	protected:
		RenderPassSpecification m_Specification;
	};
} // namespace Neon
