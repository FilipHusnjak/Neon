#pragma once

#include "Neon/Core/Application.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Renderer/RenderPass.h"
#include "Neon/Renderer/RendererAPI.h"
#include "Neon/Renderer/RendererContext.h"

namespace Neon
{
	class Renderer
	{
	public:
		static void Init();

		static void Begin();

		static void BeginRenderPass(const SharedRef<RenderPass>& renderPass);

		static void SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform);

		static void SubmitFullscreenQuad(const SharedRef<GraphicsPipeline>& graphicsPipeline);

		static void EndRenderPass();

		static void End();

		static void* GetFinalImageId();

		static void Shutdown();

		static RendererAPI::API GetAPI()
		{
			return RendererAPI::Current();
		}

		static SharedRef<RendererContext> GetContext()
		{
			return Application::Get().GetWindow().GetRenderContext();
		}

	private:
		static UniqueRef<RendererAPI> s_RendererAPI;
	};
} // namespace Neon
