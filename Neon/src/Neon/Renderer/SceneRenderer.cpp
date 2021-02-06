#include "neopch.h"

#include "Neon/Renderer/Framebuffer.h"
#include "Neon/Renderer/Renderer.h"
#include "Neon/Scene/Components.h"
#include "SceneRenderer.h"

namespace Neon
{
	struct SceneRendererData
	{
		Scene* ActiveScene = nullptr;
		struct SceneInfo
		{
			SceneRendererCamera SceneCamera;
			std::string EnvironmentPath;
		} SceneData;

		SharedRef<RenderPass> GeoPass;

		SharedRef<Material> SkyboxMaterial;
		SharedRef<GraphicsPipeline> SkyboxGraphicsPipeline;

		glm::vec2 FocusPoint = {0.5f, 0.5f};

		struct MeshDrawCommand
		{
			SharedRef<Mesh> Mesh;
			glm::mat4 Transform;
		};

		std::vector<MeshDrawCommand> MeshDrawList;
	};

	static SceneRendererData s_Data;

	static SharedRef<Shader> s_EnvUnfilteredComputeShader;
	static SharedRef<ComputePipeline> s_EnvUnfilteredComputePipeline;

	static SharedRef<TextureCube> s_EnvironmentTextureCube;

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

		ShaderSpecification computeShaderSpecification;
		computeShaderSpecification.ShaderPaths[ShaderType::Compute] = "assets/shaders/EquirectangularToCubeMap_Compute.glsl";
		s_EnvUnfilteredComputeShader = Shader::Create(computeShaderSpecification);
		ComputePipelineSpecification computePipelineSpecification;
		computePipelineSpecification.Shader = s_EnvUnfilteredComputeShader;
		s_EnvUnfilteredComputePipeline = ComputePipeline::Create(computePipelineSpecification);
	}

	void SceneRenderer::InitializeScene(Scene* scene)
	{
		s_Data.SceneData = {};

		s_Data.ActiveScene = scene;

		CreateEnvironmentMap(scene->m_EnvironmentPath);

		ShaderSpecification skyboxShaderSpec;
		skyboxShaderSpec.ShaderPaths[ShaderType::Vertex] = "assets/shaders/Skybox_Vert.glsl";
		skyboxShaderSpec.ShaderPaths[ShaderType::Fragment] = "assets/shaders/Skybox_Frag.glsl";
		skyboxShaderSpec.VBLayout = std::vector<VertexBufferElement>{{ShaderDataType::Float2}};
		s_Data.SkyboxMaterial = SharedRef<Material>::Create(Shader::Create(skyboxShaderSpec));
		s_Data.SkyboxMaterial->SetTextureCube("u_Cubemap", 0, s_EnvironmentTextureCube);

		GraphicsPipelineSpecification skyboxGraphicsPipelineSpec;
		skyboxGraphicsPipelineSpec.Pass = s_Data.GeoPass;
		skyboxGraphicsPipelineSpec.Shader = s_Data.SkyboxMaterial->GetShader();
		s_Data.SkyboxGraphicsPipeline = GraphicsPipeline::Create(skyboxGraphicsPipelineSpec);
	}

	void SceneRenderer::SetViewportSize(uint32 width, uint32 height)
	{
		// TODO: Implement
	}

	void SceneRenderer::BeginScene(const SceneRendererCamera& camera)
	{
		NEO_CORE_ASSERT(s_Data.ActiveScene, "Scene not initialized!");

		s_Data.SceneData.SceneCamera = camera;
	}

	void SceneRenderer::EndScene()
	{
		NEO_CORE_ASSERT(s_Data.ActiveScene, "");

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

	void SceneRenderer::CreateEnvironmentMap(const std::string& filepath)
	{
		const uint32 faceSize = 2048;

		SharedRef<Texture2D> envMap = Texture2D::Create(filepath, TextureType::HDR);
		NEO_CORE_ASSERT(envMap->GetFormat() == TextureFormat::RGBAFloat16, "Image has to be HDR!");
		s_EnvUnfilteredComputeShader->SetTexture2D("u_EquirectangularTex", 0, envMap);

		s_EnvironmentTextureCube = TextureCube::Create(faceSize, TextureType::HDR);
		s_EnvUnfilteredComputeShader->SetStorageTextureCube("o_CubeMap", 0, s_EnvironmentTextureCube);

		Renderer::DispatchCompute(s_EnvUnfilteredComputePipeline, faceSize / 32, faceSize / 32, 6);
	}

	void SceneRenderer::Shutdown()
	{
		s_Data = {};

		s_EnvironmentTextureCube.Reset();
		s_EnvUnfilteredComputePipeline.Reset();
		s_EnvUnfilteredComputeShader.Reset();
	}

	void SceneRenderer::FlushDrawList()
	{
		GeometryPass();

		s_Data.MeshDrawList.clear();
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginRenderPass(s_Data.GeoPass);

		auto& sceneCamera = s_Data.SceneData.SceneCamera;

		// Using vec4 for shader alignment!
		struct
		{
			glm::uvec4 Count = {0, 0, 0, 0};
			struct
			{
				glm::vec4 Strength;
				glm::vec4 Direction;
				glm::vec4 Radiance;
			} Lights[100];
		} lightUBO = {};

		uint32 i = 0;
		for (auto entity : s_Data.ActiveScene->GetAllEntitiesWithComponent<LightComponent>())
		{
			if (i >= 100)
			{
				NEO_CORE_ASSERT("Max number of light entities in the scene is {}", 100);
			}
			const auto& lightComponent = s_Data.ActiveScene->GetEntityComponent<LightComponent>(entity);
			lightUBO.Lights[i].Strength[0] = lightComponent.Strength;
			lightUBO.Lights[i].Direction = lightComponent.Direction;
			lightUBO.Lights[i].Radiance = lightComponent.Radiance;
			i++;
		}
		lightUBO.Count[0] = i;

		uint32 sizeToUpdate = sizeof(glm::vec4) + i * sizeof(lightUBO.Lights[0]);

		struct
		{
			glm::mat4 Model = glm::mat4(1.f);
			glm::mat4 ViewProjection = glm::mat4(1.f);
			glm::vec4 CameraPosition = glm::vec4();
		} cameraUBO;
		cameraUBO.ViewProjection = sceneCamera.Camera.GetViewProjection();
		cameraUBO.CameraPosition = glm::vec4(sceneCamera.Camera.GetPosition(), 1.f);

		// Render meshes
		for (auto& dc : s_Data.MeshDrawList)
		{
			cameraUBO.Model = dc.Transform;

			SharedRef<Shader> meshShader = dc.Mesh->GetShader();
			meshShader->SetUniformBuffer("CameraUBO", 0, &cameraUBO);
			meshShader->SetUniformBuffer("LightUBO", 0, &lightUBO);
			Renderer::SubmitMesh(dc.Mesh, dc.Transform);
		}

		glm::mat4 viewRotation = sceneCamera.Camera.GetViewMatrix();
		viewRotation[3][0] = 0;
		viewRotation[3][1] = 0;
		viewRotation[3][2] = 0;
		glm::mat4 inverseVP = glm::inverse(sceneCamera.Camera.GetProjectionMatrix() * viewRotation);
		s_Data.SkyboxMaterial->GetShader()->SetUniformBuffer("CameraUBO", 0, &inverseVP);
		Renderer::SubmitFullscreenQuad(s_Data.SkyboxGraphicsPipeline);

		Renderer::EndRenderPass();
	}

} // namespace Neon
