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

	const std::vector<glm::vec3> WaterVertices = {{1, 0, 1},  {0, 1, 0}, {1, 0, -1},  {0, 1, 0},
												  {-1, 0, 1}, {0, 1, 0}, {-1, 0, -1}, {0, 1, 0}};
	const std::vector<uint32> WaterIndices = {0, 1, 2, 2, 1, 3};

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

		// Water data
		SharedRef<Texture2D> DudvMap;
		SharedRef<Texture2D> NormalMap;
		SharedRef<Texture2D> Reflection;
		SharedRef<Texture2D> Refraction;
		SharedRef<RenderPass> WaterPass;
		SharedRef<Shader> WaterShader;
		SharedRef<Pipeline> WaterPipeline;
		SharedRef<VertexBuffer> WaterVertexBuffer;
		SharedRef<IndexBuffer> WaterIndexBuffer;

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

		RenderPassSpecification waterRenderPassSpec;
		s_Data.WaterPass = RenderPass::Create(waterRenderPassSpec);

		FramebufferSpecification geoFramebufferSpec;
		geoFramebufferSpec.Pass = s_Data.GeoPass.Ptr();
		geoFramebufferSpec.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};

		FramebufferSpecification waterFramebufferSpec;
		waterFramebufferSpec.Pass = s_Data.WaterPass.Ptr();
		waterFramebufferSpec.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};

		s_Data.GeoPass->SetTargetFramebuffer(Framebuffer::Create(geoFramebufferSpec));
		s_Data.WaterPass->SetTargetFramebuffer(Framebuffer::Create(waterFramebufferSpec));

		std::unordered_map<ShaderType, std::string> waterShaderPaths;
		waterShaderPaths[ShaderType::Vertex] = "assets/shaders/water_vert.glsl";
		waterShaderPaths[ShaderType::Fragment] = "assets/shaders/water_frag.glsl";

		ShaderSpecification waterShaderSpec;
		s_Data.WaterShader = Shader::Create(waterShaderSpec, waterShaderPaths);

		s_Data.DudvMap = Texture2D::Create("assets/textures/water/dudvMap.png");
		s_Data.NormalMap = Texture2D::Create("assets/textures/water/normalMap.png");

		s_Data.WaterShader->SetTexture(3, 0, s_Data.DudvMap);
		s_Data.WaterShader->SetTexture(4, 0, s_Data.NormalMap);

		VertexBufferLayout waterVertexBufferLayout =
			std::vector<VertexBufferElement>{{ShaderDataType::Float3}, {ShaderDataType::Float3}};

		s_Data.WaterVertexBuffer = VertexBuffer::Create(
			WaterVertices.data(), static_cast<uint32>(WaterVertices.size()) * sizeof(WaterVertices[0]), waterVertexBufferLayout);
		s_Data.WaterIndexBuffer =
			IndexBuffer::Create(WaterIndices.data(), static_cast<uint32>(WaterIndices.size()) * sizeof(WaterIndices[0]));

		PipelineSpecification waterPipelineSpec;
		waterPipelineSpec.Layout = waterVertexBufferLayout;
		waterPipelineSpec.Pass = s_Data.WaterPass;
		waterPipelineSpec.Shader = s_Data.WaterShader;
		s_Data.WaterPipeline = Pipeline::Create(waterPipelineSpec);
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
