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

		// Skybox
		SharedRef<TextureCube> SkyboxCubemap;
		SharedRef<Shader> SkyboxShader;
		SharedRef<Pipeline> SkyboxPipeline;

		float LightDistance = 0.1f;
		glm::mat4 LightMatrices[4];
		glm::mat4 LightViewMatrix;

		glm::vec2 FocusPoint = {0.5f, 0.5f};

		struct MeshDrawCommand
		{
			SharedRef<Mesh> Mesh;
			glm::mat4 Transform;
		};

		std::vector<MeshDrawCommand> MeshDrawList;
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

		std::unordered_map<ShaderType, std::string> skyboxShaderPaths;
		skyboxShaderPaths[ShaderType::Vertex] = "assets/shaders/skybox_vert.glsl";
		skyboxShaderPaths[ShaderType::Fragment] = "assets/shaders/skybox_frag.glsl";
		ShaderSpecification skyboxShaderSpec;
		s_Data.SkyboxShader = Shader::Create(skyboxShaderSpec, skyboxShaderPaths);

		s_Data.DudvMap = Texture2D::Create("assets/textures/water/dudvMap.png");
		s_Data.NormalMap = Texture2D::Create("assets/textures/water/normalMap.png");

		s_Data.WaterShader->SetTexture2D(3, 0, s_Data.DudvMap);
		s_Data.WaterShader->SetTexture2D(4, 0, s_Data.NormalMap);

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

		VertexBufferLayout skyboxVertexBufferLayout = std::vector<VertexBufferElement>{{ShaderDataType::Float2}};

		PipelineSpecification skyboxPipelineSpec;
		skyboxPipelineSpec.Layout = skyboxVertexBufferLayout;
		skyboxPipelineSpec.Pass = s_Data.GeoPass;
		skyboxPipelineSpec.Shader = s_Data.SkyboxShader;
		s_Data.SkyboxPipeline = Pipeline::Create(skyboxPipelineSpec);

		s_Data.SkyboxCubemap = TextureCube::Create({"assets/textures/skybox/meadow/posz.jpg", "assets/textures/skybox/meadow/negz.jpg",
													"assets/textures/skybox/meadow/posy.jpg", "assets/textures/skybox/meadow/negy.jpg",
													"assets/textures/skybox/meadow/negx.jpg", "assets/textures/skybox/meadow/posx.jpg"});
		s_Data.SkyboxShader->SetTextureCube(1, 0, s_Data.SkyboxCubemap);
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
		s_Data.MeshDrawList.push_back({mesh, transform});
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

		s_Data.MeshDrawList.clear();
		s_Data.SceneData = {};
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginRenderPass(s_Data.GeoPass);

		auto& sceneCamera = s_Data.SceneData.SceneCamera;

		auto viewProjection = sceneCamera.Camera.GetProjectionMatrix() * sceneCamera.ViewMatrix;

		// Render meshes
		for (auto& dc : s_Data.MeshDrawList)
		{
			SharedRef<Shader> meshShader = dc.Mesh->GetShader();
			CameraMatrices cameraMatrices = {};
			cameraMatrices.ViewProjection = viewProjection;
			cameraMatrices.Model = dc.Transform;
			meshShader->SetUniformBuffer(0, 0, &cameraMatrices);
			Renderer::SubmitMesh(dc.Mesh, dc.Transform);
		}
		glm::mat4 viewRotation = sceneCamera.ViewMatrix;
		viewRotation[3][0] = 0;
		viewRotation[3][1] = 0;
		viewRotation[3][2] = 0;
		glm::mat4 inverseVP = glm::inverse(sceneCamera.Camera.GetProjectionMatrix() * viewRotation);
		s_Data.SkyboxShader->SetUniformBuffer(0, 0, &inverseVP);
		Renderer::SubmitFullscreenQuad(s_Data.SkyboxPipeline);

		Renderer::EndRenderPass();
	}

} // namespace Neon
