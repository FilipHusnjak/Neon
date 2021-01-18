#include "neopch.h"

#include "Neon/Renderer/Framebuffer.h"
#include "Neon/Renderer/Renderer.h"
#include "SceneRenderer.h"

namespace Neon
{
	// TODO: temp
	struct CameraMatrices
	{
		glm::mat4 Model = glm::mat4(1.f);
		glm::mat4 ViewProjection = glm::mat4(1.f);
	};

	struct SceneRendererData
	{
		const Scene* ActiveScene = nullptr;
		struct SceneInfo
		{
			SceneRendererCamera SceneCamera;

			// Resources
			Light ActiveLight;
		} SceneData;

		SharedRef<RenderPass> GeoPass;

		float LightDistance = 0.1f;
		glm::mat4 LightMatrices[4];
		glm::mat4 LightViewMatrix;

		glm::vec2 FocusPoint = {0.5f, 0.5f};

		struct DrawCommand
		{
			SharedRef<Mesh> Mesh;
			glm::mat4 Transform;
		};

		std::vector<DrawCommand> DrawList;
	};

	static SceneRendererData s_Data;

	void SceneRenderer::Init()
	{
		RenderPassSpecification geoRenderPassSpec;
		s_Data.GeoPass = RenderPass::Create(geoRenderPassSpec);

		FramebufferSpecification geoFramebufferSpec;
		geoFramebufferSpec.Pass = s_Data.GeoPass.Ptr();
		geoFramebufferSpec.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};

		s_Data.GeoPass->SetTargetFramebuffer(Framebuffer::Create(geoFramebufferSpec));
	}

	void SceneRenderer::SetViewportSize(uint32 width, uint32 height)
	{
		s_Data.GeoPass->GetSpecification().TargetFramebuffer->Resize(width, height);
	}

	void SceneRenderer::BeginScene(const Scene* scene, const SceneRendererCamera& camera)
	{
		NEO_CORE_ASSERT(!s_Data.ActiveScene, "Another scene is currently rendering!");

		s_Data.ActiveScene = scene;

		s_Data.SceneData.SceneCamera = camera;
		s_Data.SceneData.ActiveLight = scene->GetLight();
	}

	void SceneRenderer::EndScene()
	{
		NEO_CORE_ASSERT(s_Data.ActiveScene, "");

		s_Data.ActiveScene = nullptr;

		FlushDrawList();
	}

	void SceneRenderer::SubmitMesh(SharedRef<Mesh> mesh, const glm::mat4& transform /*= glm::mat4(1.0f)*/)
	{
		s_Data.DrawList.push_back({mesh, transform});
	}

	const SharedRef<RenderPass>& SceneRenderer::GetGeoPass()
	{
		return s_Data.GeoPass;
	}

	void* SceneRenderer::GetFinalColorBufferRendererId()
	{
		return s_Data.GeoPass->GetSpecification().TargetFramebuffer->GetColorImageID();
	}

	void SceneRenderer::SetFocusPoint(const glm::vec2& point)
	{
		s_Data.FocusPoint = point;
	}

	void SceneRenderer::OnImGuiRender()
	{
	}

	void SceneRenderer::Shutdown()
	{
		s_Data = {};
	}

	void SceneRenderer::FlushDrawList()
	{
		GeometryPass();

		s_Data.DrawList.clear();
		s_Data.SceneData = {};
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginRenderPass(s_Data.GeoPass);

		auto& sceneCamera = s_Data.SceneData.SceneCamera;

		auto viewProjection = sceneCamera.Camera.GetProjectionMatrix() * sceneCamera.ViewMatrix;

		// Render meshes
		for (auto& dc : s_Data.DrawList)
		{
			SharedRef<Shader> meshShader = dc.Mesh->GetShader();
			CameraMatrices cameraMatrices = {};
			cameraMatrices.ViewProjection = viewProjection;
			cameraMatrices.Model = dc.Transform;
			meshShader->SetUniformBuffer(0, 0, &cameraMatrices);
			Renderer::SubmitMesh(dc.Mesh, dc.Transform);
		}

		Renderer::EndRenderPass();
	}

} // namespace Neon
