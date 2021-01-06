#pragma once

#include "RenderPass.h"

#include <glm/glm.hpp>

namespace Neon
{
	struct FramebufferSpecification
	{
		uint32 Width = 1920;
		uint32 Height = 1080;
		SharedRef<RenderPass> Pass;
		glm::vec4 ClearColor = {1.f, 1.f, 1.f, 1.f};
	};

	class Framebuffer : public RefCounted
	{
	public:
		Framebuffer(const FramebufferSpecification& spec);
		virtual ~Framebuffer() = default;

		virtual void Resize(uint32 width, uint32 height, bool forceRecreate = false) = 0;

		const FramebufferSpecification& GetSpecification() const
		{
			return m_Specification;
		}

		virtual void* GetColorImageID() const = 0;

		static SharedRef<Framebuffer> Create(const FramebufferSpecification& spec);

	protected:
		FramebufferSpecification m_Specification;
	};

	class FramebufferPool final
	{
	public:
		FramebufferPool() = default;
		~FramebufferPool() = default;

		void Add(Framebuffer* framebuffer);

		std::vector<Framebuffer*>& GetAll()
		{
			return m_Framebuffers;
		}
		const std::vector<Framebuffer*>& GetAll() const
		{
			return m_Framebuffers;
		}

		inline static FramebufferPool& Get()
		{
			return *s_Instance;
		}

	private:
		std::vector<Framebuffer*> m_Framebuffers;

		static FramebufferPool* s_Instance;
	};
} // namespace Neon
