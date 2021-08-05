#include "neopch.h"

#include "Neon/Renderer/Framebuffer.h"
#include "Neon/Renderer/Renderer.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Components/LightComponent.h"

namespace Neon
{
	struct SceneRendererData
	{
		SharedRef<Scene> ActiveScene = nullptr;
		SharedRef<Actor> SelectedActor = {};

		std::vector<Light> Lights;

		struct SceneInfo
		{
			SceneRendererCamera SceneCamera;
			std::string EnvironmentPath;
		} SceneData;

		SharedRef<RenderPass> GeoPass;
		SharedRef<RenderPass> PostProcessingPass;

		SharedRef<Shader> PostProcessingShader;
		SharedRef<GraphicsPipeline> PostProcessingPipeline;

		SharedRef<TextureCube> EnvUnfilteredTextureCube;
		SharedRef<TextureCube> EnvFilteredTextureCube;
		SharedRef<TextureCube> IrradianceTextureCube;

		glm::vec2 FocusPoint = {0.5f, 0.5f};

		struct MeshDrawCommand
		{
			SharedRef<Mesh> Mesh;
			glm::mat4 Transform;
		};

		std::vector<MeshDrawCommand> MeshDrawList;

		Material SkyboxMaterial;
		SharedRef<GraphicsPipeline> SkyboxGraphicsPipeline;

		SharedRef<Shader> EnvUnfilteredComputeShader;
		SharedRef<ComputePipeline> EnvUnfilteredComputePipeline;

		SharedRef<Shader> EnvFilteredComputeShader;
		SharedRef<ComputePipeline> EnvFilteredComputePipeline;

		SharedRef<Shader> IrradianceComputeShader;
		SharedRef<ComputePipeline> IrradianceComputePipeline;

		SharedRef<Texture2D> BRDFLUT;

		float CurrentTime = 0;
	};

	static SceneRendererData s_DebuggerData;

	void SceneRenderer::Init()
	{
		uint32 width = Application::Get().GetWindow().GetWidth();
		uint32 height = Application::Get().GetWindow().GetWidth();

		{
			RenderPassSpecification geoRenderPassSpec;
			geoRenderPassSpec.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};
			geoRenderPassSpec.Attachments.push_back(
				{4, TextureFormat::RGBA16F, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, false});
			geoRenderPassSpec.Attachments.push_back(
				{1, TextureFormat::RGBA16F, AttachmentLoadOp::DontCare, AttachmentStoreOp::Store, true});
			geoRenderPassSpec.Attachments.push_back(
				{4, TextureFormat::Depth, AttachmentLoadOp::Clear, AttachmentStoreOp::DontCare, false});
			geoRenderPassSpec.Subpasses.push_back({true, {}, {0}, {1}});
			s_DebuggerData.GeoPass = RenderPass::Create(geoRenderPassSpec);
			FramebufferSpecification geoFramebufferSpec;
			geoFramebufferSpec.Width = width;
			geoFramebufferSpec.Height = height;
			geoFramebufferSpec.Pass = s_DebuggerData.GeoPass.Ptr();
			s_DebuggerData.GeoPass->SetTargetFramebuffer(Framebuffer::Create(geoFramebufferSpec));
		}

		{
			RenderPassSpecification postProcessingPassSpec;
			postProcessingPassSpec.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};
			postProcessingPassSpec.Attachments.push_back(
				{1, TextureFormat::RGBA8, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, true});
			postProcessingPassSpec.Subpasses.push_back({false, {}, {0}, {}});
			s_DebuggerData.PostProcessingPass = RenderPass::Create(postProcessingPassSpec);
			FramebufferSpecification postProcessingFramebufferSpec;
			postProcessingFramebufferSpec.Width = width;
			postProcessingFramebufferSpec.Height = height;
			postProcessingFramebufferSpec.Pass = s_DebuggerData.PostProcessingPass.Ptr();
			s_DebuggerData.PostProcessingPass->SetTargetFramebuffer(Framebuffer::Create(postProcessingFramebufferSpec));
		}

		{
			ShaderSpecification postProcessingShaderSpecification;
			postProcessingShaderSpecification.ShaderPaths[ShaderType::Vertex] = "assets/shaders/PostProcess_Vert.glsl";
			postProcessingShaderSpecification.ShaderPaths[ShaderType::Fragment] = "assets/shaders/PostProcess_Frag.glsl";
			postProcessingShaderSpecification.VBLayout = std::vector<VertexBufferElement>{{ShaderDataType::Float2}};
			s_DebuggerData.PostProcessingShader = Shader::Create(postProcessingShaderSpecification);
			GraphicsPipelineSpecification postProcessingPipelineSpecification;
			postProcessingPipelineSpecification.Pass = s_DebuggerData.PostProcessingPass;
			s_DebuggerData.PostProcessingPipeline =
				GraphicsPipeline::Create(s_DebuggerData.PostProcessingShader, postProcessingPipelineSpecification);
		}

		{
			ShaderSpecification envUnfilteredComputeShaderSpecification;
			envUnfilteredComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/EquirectangularToCubeMap_Compute.glsl";
			s_DebuggerData.EnvUnfilteredComputeShader = Shader::Create(envUnfilteredComputeShaderSpecification);
			ComputePipelineSpecification envUnfilteredPipelineSpecification;
			s_DebuggerData.EnvUnfilteredComputePipeline =
				ComputePipeline::Create(s_DebuggerData.EnvUnfilteredComputeShader, envUnfilteredPipelineSpecification);
		}

		{
			ShaderSpecification envFilteredComputeShaderSpecification;
			envFilteredComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/EnvironmentMipFilter_Compute.glsl";
			s_DebuggerData.EnvFilteredComputeShader = Shader::Create(envFilteredComputeShaderSpecification);
			ComputePipelineSpecification envFilteredComputePipelineSpecification;
			s_DebuggerData.EnvFilteredComputePipeline =
				ComputePipeline::Create(s_DebuggerData.EnvFilteredComputeShader, envFilteredComputePipelineSpecification);
		}

		{
			ShaderSpecification irradianceComputeShaderSpecification;
			irradianceComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/EnvironmentIrradiance_Compute.glsl";
			s_DebuggerData.IrradianceComputeShader = Shader::Create(irradianceComputeShaderSpecification);
			ComputePipelineSpecification irradianceComputePipelineSpecification;
			s_DebuggerData.IrradianceComputePipeline =
				ComputePipeline::Create(s_DebuggerData.IrradianceComputeShader, irradianceComputePipelineSpecification);
		}
	}

	void SceneRenderer::InitializeScene(SharedRef<Scene> scene)
	{
		NEO_CORE_ASSERT(scene);

		s_DebuggerData.SceneData = {};

		s_DebuggerData.ActiveScene = scene;

		CreateEnvironmentMap(scene->m_EnvironmentPath);

		ShaderSpecification skyboxShaderSpec;
		skyboxShaderSpec.ShaderPaths[ShaderType::Vertex] = "assets/shaders/Skybox_Vert.glsl";
		skyboxShaderSpec.ShaderPaths[ShaderType::Fragment] = "assets/shaders/Skybox_Frag.glsl";
		skyboxShaderSpec.VBLayout = std::vector<VertexBufferElement>{{ShaderDataType::Float2}};
		s_DebuggerData.SkyboxMaterial = Material(0, Shader::Create(skyboxShaderSpec));
		s_DebuggerData.SkyboxMaterial.SetTextureCube("u_Cubemap", s_DebuggerData.EnvFilteredTextureCube, 0);

		GraphicsPipelineSpecification skyboxGraphicsPipelineSpec;
		skyboxGraphicsPipelineSpec.Pass = s_DebuggerData.GeoPass;
		s_DebuggerData.SkyboxGraphicsPipeline = GraphicsPipeline::Create(s_DebuggerData.SkyboxMaterial.GetShader(), skyboxGraphicsPipelineSpec);
	}

	const SharedRef<Scene>& SceneRenderer::GetActiveScene()
	{
		return s_DebuggerData.ActiveScene;
	}

	void SceneRenderer::SetSelectedActor(SharedRef<Actor> actor)
	{
		s_DebuggerData.SelectedActor = actor;
	}

	SharedRef<Actor> SceneRenderer::GetSelectedActor()
	{
		return s_DebuggerData.SelectedActor;
	}

	SharedRef<Actor> SceneRenderer::CreateActor(UUID uuid, const std::string& name /*= "Actor"*/)
	{
		NEO_CORE_ASSERT(s_DebuggerData.ActiveScene);

		return s_DebuggerData.ActiveScene->CreateActor(uuid, name);
	}

	void SceneRenderer::DestroyActor(SharedRef<Actor> actor)
	{
		NEO_CORE_ASSERT(s_DebuggerData.ActiveScene);

		s_DebuggerData.ActiveScene->DestroyActor(actor);
	}

	void SceneRenderer::SetViewportSize(uint32 width, uint32 height)
	{
		// TODO: Implement
	}

	void SceneRenderer::BeginScene(const SceneRendererCamera& camera)
	{
		NEO_CORE_ASSERT(s_DebuggerData.ActiveScene, "Scene not initialized!");

		s_DebuggerData.SceneData.SceneCamera = camera;
	}

	void SceneRenderer::EndScene()
	{
		NEO_CORE_ASSERT(s_DebuggerData.ActiveScene, "");

		FlushDrawList();
	}

	void SceneRenderer::SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform /*= glm::mat4(1.0f)*/)
	{
		s_DebuggerData.MeshDrawList.push_back({mesh, transform});
	}

	void SceneRenderer::SubmitLight(const Light& light)
	{
		s_DebuggerData.Lights.push_back(light);
	}

	const SharedRef<RenderPass>& SceneRenderer::GetGeoPass()
	{
		return s_DebuggerData.GeoPass;
	}

	const SharedRef<TextureCube>& SceneRenderer::GetRadianceTex()
	{
		return s_DebuggerData.EnvFilteredTextureCube;
	}

	const SharedRef<TextureCube>& SceneRenderer::GetIrradianceTex()
	{
		return s_DebuggerData.IrradianceTextureCube;
	}

	const SharedRef<Texture2D>& SceneRenderer::GetBRDFLUTTex()
	{
		return s_DebuggerData.BRDFLUT;
	}

	void SceneRenderer::SetFocusPoint(const glm::vec2& point)
	{
		s_DebuggerData.FocusPoint = point;
	}

	void* SceneRenderer::GetFinalImageId()
	{
		return s_DebuggerData.PostProcessingPass->GetTargetFramebuffer()->GetSampledImageId();
	}

	void SceneRenderer::OnImGuiRender()
	{
	}

	void SceneRenderer::CreateEnvironmentMap(const std::string& filepath)
	{
		const uint32 faceSize = 2048;
		const uint32 irradianceMapSize = 32;

		SharedRef<Texture2D> envMap =
			Texture2D::Create(filepath, {TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA16F, TextureWrap::Clamp});
		NEO_CORE_ASSERT(envMap->GetFormat() == TextureFormat::RGBA16F, "Image has to be HDR!");
		s_DebuggerData.EnvUnfilteredComputeShader->SetTexture2D("u_EquirectangularTex", 0, envMap, 0);

		s_DebuggerData.EnvUnfilteredTextureCube =
			TextureCube::Create({TextureUsageFlagBits::ShaderWrite | TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA16F,
								 TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, true, faceSize, faceSize});
		s_DebuggerData.EnvFilteredTextureCube =
			TextureCube::Create({TextureUsageFlagBits::ShaderWrite | TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA16F,
								 TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, true, faceSize, faceSize});

		s_DebuggerData.EnvUnfilteredComputeShader->SetStorageTextureCube("o_CubeMap", 0, s_DebuggerData.EnvUnfilteredTextureCube, 0);
		Renderer::DispatchCompute(s_DebuggerData.EnvUnfilteredComputePipeline, faceSize / 32, faceSize / 32, 6);
		s_DebuggerData.EnvUnfilteredTextureCube->RegenerateMipMaps();

		s_DebuggerData.EnvUnfilteredComputeShader->SetStorageTextureCube("o_CubeMap", 0, s_DebuggerData.EnvFilteredTextureCube, 0);
		Renderer::DispatchCompute(s_DebuggerData.EnvUnfilteredComputePipeline, faceSize / 32, faceSize / 32, 6);
		s_DebuggerData.EnvFilteredTextureCube->RegenerateMipMaps();

		s_DebuggerData.EnvFilteredComputeShader->SetTextureCube("u_InputCubemap", 0, s_DebuggerData.EnvUnfilteredTextureCube, 0);
		for (uint32 level = 1, size = faceSize; level < s_DebuggerData.EnvUnfilteredTextureCube->GetMipLevelCount(); level++, size /= 2)
		{
			uint32 test = s_DebuggerData.EnvFilteredTextureCube->GetMipLevelCount();
			const uint32 numGroups = glm::max(1u, size / 32);
			struct
			{
				float MipCount;
				float MipLevel;
			} pc = {static_cast<float>(s_DebuggerData.EnvFilteredTextureCube->GetMipLevelCount()), static_cast<float>(level)};
			s_DebuggerData.EnvFilteredComputeShader->SetPushConstant("u_PushConstant", &pc);
			s_DebuggerData.EnvFilteredComputeShader->SetStorageTextureCube("o_OutputCubemap", 0, s_DebuggerData.EnvFilteredTextureCube, level);
			Renderer::DispatchCompute(s_DebuggerData.EnvFilteredComputePipeline, numGroups, numGroups, 6);
		}

		s_DebuggerData.IrradianceTextureCube = TextureCube::Create({TextureUsageFlagBits::ShaderWrite | TextureUsageFlagBits::ShaderRead,
															TextureFormat::RGBA16F, TextureWrap::Clamp, TextureMinMagFilter::Linear,
															true, 1, true, irradianceMapSize, irradianceMapSize});
		s_DebuggerData.IrradianceComputeShader->SetTextureCube("u_InputCubemap", 0, s_DebuggerData.EnvFilteredTextureCube, 0);
		s_DebuggerData.IrradianceComputeShader->SetStorageTextureCube("o_OutputCubemap", 0, s_DebuggerData.IrradianceTextureCube, 0);
		Renderer::DispatchCompute(s_DebuggerData.IrradianceComputePipeline, irradianceMapSize / 32, irradianceMapSize / 32, 6);
		s_DebuggerData.IrradianceTextureCube->RegenerateMipMaps();

		s_DebuggerData.BRDFLUT = Texture2D::Create("assets/textures/environment/BRDF_LUT.tga",
										   {TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA8, TextureWrap::Clamp});
	}

	void SceneRenderer::Shutdown()
	{
		s_DebuggerData = {};
	}

	void SceneRenderer::FlushDrawList()
	{
		GeometryPass();
		PostProcessingPass();
		s_DebuggerData.MeshDrawList.clear();
		s_DebuggerData.Lights.clear();
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginRenderPass(s_DebuggerData.GeoPass);

		auto& sceneCamera = s_DebuggerData.SceneData.SceneCamera;

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
		for (auto& light : s_DebuggerData.Lights)
		{
			if (i >= 100)
			{
				NEO_CORE_ASSERT("Max number of light entities in the scene is {}", 100);
			}
			lightUBO.Lights[i].Strength[0] = light.Strength;
			lightUBO.Lights[i].Direction = light.Direction;
			lightUBO.Lights[i].Radiance = light.Radiance;
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
		for (auto& dc : s_DebuggerData.MeshDrawList)
		{
			cameraUBO.Model = dc.Transform;

			if (Renderer::IsWireframeEnabled())
			{
				SharedRef<Shader> meshShader = dc.Mesh->GetWireframeShader();
				meshShader->SetUniformBuffer("CameraUBO", 0, &cameraUBO);
			}
			else
			{
				SharedRef<Shader> meshShader = dc.Mesh->GetShader();
				meshShader->SetUniformBuffer("CameraUBO", 0, &cameraUBO);
				meshShader->SetUniformBuffer("LightUBO", 0, &lightUBO);
			}

			Renderer::SubmitMesh(dc.Mesh, dc.Transform);
		}

		glm::mat4 viewRotation = sceneCamera.Camera.GetViewMatrix();
		viewRotation[3][0] = 0;
		viewRotation[3][1] = 0;
		viewRotation[3][2] = 0;
		glm::mat4 inverseVP = glm::inverse(sceneCamera.Camera.GetProjectionMatrix() * viewRotation);
		s_DebuggerData.SkyboxMaterial.GetShader()->SetUniformBuffer("CameraUBO", 0, &inverseVP);
		Renderer::SubmitFullscreenQuad(s_DebuggerData.SkyboxGraphicsPipeline);

		Renderer::EndRenderPass();
	}

	void SceneRenderer::PostProcessingPass()
	{
		Renderer::BeginRenderPass(s_DebuggerData.PostProcessingPass);
		s_DebuggerData.PostProcessingShader->SetTexture2D("u_Texture", 0, s_DebuggerData.GeoPass->GetTargetFramebuffer()->GetSampledImage(), 0);
		Renderer::SubmitFullscreenQuad(s_DebuggerData.PostProcessingPipeline);
		Renderer::EndRenderPass();
	}

} // namespace Neon
