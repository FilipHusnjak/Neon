#include "neopch.h"

#include "Renderer.h"

namespace Neon
{
	void Renderer::Init()
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->Init();
	}

	void Renderer::Render(SharedRef<PerspectiveCameraController>& camera)
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->Render(camera);
	}

	void Renderer::Shutdown()
	{
		NEO_CORE_ASSERT(s_RendererAPI, "Renderer API not selected!");
		s_RendererAPI->Shutdown();
	}

	UniqueRef<RendererAPI> Renderer::s_RendererAPI = RendererAPI::Create();
}
