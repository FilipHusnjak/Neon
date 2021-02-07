#pragma once

#include <glm/glm.hpp>

namespace Neon
{
	class RenderPass;

	struct FramebufferSpecification
	{
		uint32 Width = 1920;
		uint32 Height = 1080;

		// Use weak ref here
		RenderPass* Pass;

		// TODO: Temp, needs scale
		bool NoResize = false;
	};

	class Framebuffer : public RefCounted
	{
	public:
		static SharedRef<Framebuffer> Create(const FramebufferSpecification& spec);

	public:
		Framebuffer(const FramebufferSpecification& spec);
		virtual ~Framebuffer() = default;

		virtual void Resize(uint32 width, uint32 height, bool forceRecreate = false) = 0;

		virtual void* GetSampledImageId() const = 0;

		const FramebufferSpecification& GetSpecification() const
		{
			return m_Specification;
		}

		virtual void* GetHandle() const = 0;

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
		// Use weak ref here
		std::vector<Framebuffer*> m_Framebuffers;

		static FramebufferPool* s_Instance;
	};
} // namespace Neon
