#include "neopch.h"

#include "Neon/Renderer/Framebuffer.h"
#include "Neon/Renderer/Renderer.h"
#include "SceneRenderer.h"

namespace Neon
{
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
		geoRenderPassSpec.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};
		geoRenderPassSpec.Attachments.push_back(
			{8, AttachmentFormat::RGBA8, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, false});
		geoRenderPassSpec.Attachments.push_back(
			{1, AttachmentFormat::RGBA8, AttachmentLoadOp::DontCare, AttachmentStoreOp::Store, true});
		geoRenderPassSpec.Attachments.push_back(
			{8, AttachmentFormat::Depth, AttachmentLoadOp::Clear, AttachmentStoreOp::DontCare, false});
		geoRenderPassSpec.Subpasses.push_back({true, {}, {0}, {1}});
		s_Data.GeoPass = RenderPass::Create(geoRenderPassSpec);

		FramebufferSpecification geoFramebufferSpec;
		geoFramebufferSpec.Pass = s_Data.GeoPass.Ptr();

		s_Data.GeoPass->SetTargetFramebuffer(Framebuffer::Create(geoFramebufferSpec));

		std::unordered_map<ShaderType, std::string> skyboxShaderPaths;
		skyboxShaderPaths[ShaderType::Vertex] = "assets/shaders/skybox_vert.glsl";
		skyboxShaderPaths[ShaderType::Fragment] = "assets/shaders/skybox_frag.glsl";
		ShaderSpecification skyboxShaderSpec;
		s_Data.SkyboxShader = Shader::Create(skyboxShaderSpec, skyboxShaderPaths);

		VertexBufferLayout waterVertexBufferLayout =
			std::vector<VertexBufferElement>{{ShaderDataType::Float3}, {ShaderDataType::Float3}};

		VertexBufferLayout skyboxVertexBufferLayout = std::vector<VertexBufferElement>{{ShaderDataType::Float2}};

		PipelineSpecification skyboxPipelineSpec;
		skyboxPipelineSpec.Layout = skyboxVertexBufferLayout;
		skyboxPipelineSpec.Pass = s_Data.GeoPass;
		skyboxPipelineSpec.Shader = s_Data.SkyboxShader;
		s_Data.SkyboxPipeline = Pipeline::Create(skyboxPipelineSpec);

		s_Data.SkyboxCubemap =
			TextureCube::Create({"assets/textures/skybox/meadow/posz.jpg", "assets/textures/skybox/meadow/negz.jpg",
								 "assets/textures/skybox/meadow/posy.jpg", "assets/textures/skybox/meadow/negy.jpg",
								 "assets/textures/skybox/meadow/negx.jpg", "assets/textures/skybox/meadow/posx.jpg"});
		s_Data.SkyboxShader->SetTextureCube(1, 0, s_Data.SkyboxCubemap);
	}

	void SceneRenderer::SetViewportSize(uint32 width, uint32 height)
	{
		// TODO: Implement
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

	void SceneRenderer::SetFocusPoint(const glm::vec2& point)
	{
		s_Data.FocusPoint = point;
	}

	void* SceneRenderer::GetFinalImageId()
	{
		return s_Data.GeoPass->GetTargetFramebuffer()->GetSampledImageId();
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
			struct
			{
				glm::mat4 Model = glm::mat4(1.f);
				glm::mat4 ViewProjection = glm::mat4(1.f);
			} cameraMatrices;
			SharedRef<Shader> meshShader = dc.Mesh->GetShader();
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
