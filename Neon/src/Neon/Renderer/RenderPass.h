#pragma once

#include "Neon/Renderer/Framebuffer.h"

namespace Neon
{
	enum class AttachmentFormat
	{
		None = 0,

		RGBA8 = 1,
		SRGBA8 = 2,
		RGBA16F = 3,
		RGBA32F = 4,
		RG32F = 5,

		Depth = 6
	};

	enum class AttachmentLoadOp
	{
		DontCare,
		Clear,
		Load
	};

	enum class AttachmentStoreOp
	{
		DontCare,
		Store
	};

	struct AttachmentSpecification
	{
		uint32 Samples = 1;
		AttachmentFormat Format = AttachmentFormat::None;
		AttachmentLoadOp LoadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp StoreOp = AttachmentStoreOp::DontCare;
		bool Sampled = false;
	};

	struct Subpass
	{
		bool EnableDepthStencil = false;
		std::vector<uint32> InputAttachments;
		std::vector<uint32> ColorAttachments;
		std::vector<uint32> ColorResolveAttachments;
	};

	struct RenderPassSpecification
	{
		glm::vec4 ClearColor = {1.f, 1.f, 1.f, 1.f};

		std::vector<AttachmentSpecification> Attachments;
		std::vector<Subpass> Subpasses;
	};

	class RenderPass : public RefCounted
	{
	public:
		RenderPass(const RenderPassSpecification& spec);
		virtual ~RenderPass() = default;

		void SetTargetFramebuffer(const SharedRef<Framebuffer>& targetFramebuffer)
		{
			m_TargetFramebuffer = targetFramebuffer;
		}
		const SharedRef<Framebuffer>& GetTargetFramebuffer() const
		{
			return m_TargetFramebuffer;
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

		SharedRef<Framebuffer> m_TargetFramebuffer;
	};
} // namespace Neon
