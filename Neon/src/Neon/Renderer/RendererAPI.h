#pragma once

#include "Neon/Core/Core.h"
#include "Neon/Renderer/Mesh.h"

namespace Neon
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			Vulkan = 1
		};

		struct RenderAPICapabilities
		{
			std::string Vendor;
			std::string Renderer;
			std::string Version;

			int MaxSamples = 0;
			float MaxAnisotropy = 0.0f;
			int MaxTextureUnits = 0;
		};

	public:
		virtual ~RendererAPI() = default;

		virtual void Init();
		virtual void Begin() = 0;
		virtual void BeginRenderPass(const SharedRef<RenderPass>& renderPass) = 0;
		virtual void SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform) = 0;
		virtual void SubmitFullscreenQuad(const SharedRef<Pipeline>& pipeline) = 0;
		virtual void EndRenderPass() = 0;
		virtual void End() = 0;
		virtual void WaitIdle() = 0;
		virtual void Shutdown();

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		static API Current()
		{
			return s_API;
		}
		static UniqueRef<RendererAPI> Create();

	protected:
		const std::vector<glm::vec2> m_QuadVertices = {{-1.f, -1.f}, {1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f}};
		const std::vector<uint32> m_QuadIndices = {0, 2, 1, 1, 2, 3};

		SharedRef<VertexBuffer> m_QuadVertexBuffer;
		SharedRef<IndexBuffer> m_QuadIndexBuffer;

	private:
		static API s_API;
	};
} // namespace Neon
