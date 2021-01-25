#include "neopch.h"

#include "Neon/Renderer/SceneRenderer.h"
#include "Renderer.h"

namespace Neon
{
	void Renderer::Init()
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->Init();
		SceneRenderer::Init();
	}

	void Renderer::Begin()
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->Begin();
	}

	void Renderer::BeginRenderPass(const SharedRef<RenderPass>& renderPass)
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->BeginRenderPass(renderPass);
	}

	void Renderer::SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform)
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->SubmitMesh(mesh, transform);
	}

	void Renderer::SubmitFullscreenQuad(const SharedRef<Pipeline>& pipeline)
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->SubmitFullscreenQuad(pipeline);
	}

	void Renderer::EndRenderPass()
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->EndRenderPass();
	}

	void Renderer::End()
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->End();
	}

	void* Renderer::GetFinalImageId()
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		return SceneRenderer::GetFinalImageId();
	}

	void Renderer::Shutdown()
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->WaitIdle();
		SceneRenderer::Shutdown();
		s_RendererAPI->Shutdown();
	}

	UniqueRef<RendererAPI> Renderer::s_RendererAPI = RendererAPI::Create();
} // namespace Neon
