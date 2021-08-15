#include "neopch.h"

#include "Neon/Renderer/Framebuffer.h"
#include "Neon/Renderer/Renderer.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Components/LightComponent.h"

namespace Neon
{
	SceneRenderer::SceneRendererData SceneRenderer::s_Data = {};

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
			s_Data.GeoPass = RenderPass::Create(geoRenderPassSpec);
			FramebufferSpecification geoFramebufferSpec;
			geoFramebufferSpec.Width = width;
			geoFramebufferSpec.Height = height;
			geoFramebufferSpec.Pass = s_Data.GeoPass.Ptr();
			s_Data.GeoPass->SetTargetFramebuffer(Framebuffer::Create(geoFramebufferSpec));
		}

		{
			RenderPassSpecification postProcessingPassSpec;
			postProcessingPassSpec.ClearColor = {0.1f, 0.1f, 0.1f, 1.0f};
			postProcessingPassSpec.Attachments.push_back(
				{1, TextureFormat::RGBA8, AttachmentLoadOp::Clear, AttachmentStoreOp::Store, true});
			postProcessingPassSpec.Subpasses.push_back({false, {}, {0}, {}});
			s_Data.PostProcessingPass = RenderPass::Create(postProcessingPassSpec);
			FramebufferSpecification postProcessingFramebufferSpec;
			postProcessingFramebufferSpec.Width = width;
			postProcessingFramebufferSpec.Height = height;
			postProcessingFramebufferSpec.Pass = s_Data.PostProcessingPass.Ptr();
			s_Data.PostProcessingPass->SetTargetFramebuffer(Framebuffer::Create(postProcessingFramebufferSpec));
		}

		{
			ShaderSpecification postProcessingShaderSpecification;
			postProcessingShaderSpecification.ShaderPaths[ShaderType::Vertex] = "assets/shaders/PostProcess_Vert.glsl";
			postProcessingShaderSpecification.ShaderPaths[ShaderType::Fragment] = "assets/shaders/PostProcess_Frag.glsl";
			postProcessingShaderSpecification.VBLayout = std::vector<VertexBufferElement>{{ShaderDataType::Float2}};
			s_Data.PostProcessingShader = Shader::Create(postProcessingShaderSpecification);
			GraphicsPipelineSpecification postProcessingPipelineSpecification;
			postProcessingPipelineSpecification.Pass = s_Data.PostProcessingPass;
			s_Data.PostProcessingPipeline =
				GraphicsPipeline::Create(s_Data.PostProcessingShader, postProcessingPipelineSpecification);
		}

		{
			ShaderSpecification envUnfilteredComputeShaderSpecification;
			envUnfilteredComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/EquirectangularToCubeMap_Compute.glsl";
			s_Data.EnvUnfilteredComputeShader = Shader::Create(envUnfilteredComputeShaderSpecification);
			ComputePipelineSpecification envUnfilteredPipelineSpecification;
			s_Data.EnvUnfilteredComputePipeline =
				ComputePipeline::Create(s_Data.EnvUnfilteredComputeShader, envUnfilteredPipelineSpecification);
		}

		{
			ShaderSpecification envFilteredComputeShaderSpecification;
			envFilteredComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/EnvironmentMipFilter_Compute.glsl";
			s_Data.EnvFilteredComputeShader = Shader::Create(envFilteredComputeShaderSpecification);
			ComputePipelineSpecification envFilteredComputePipelineSpecification;
			s_Data.EnvFilteredComputePipeline =
				ComputePipeline::Create(s_Data.EnvFilteredComputeShader, envFilteredComputePipelineSpecification);
		}

		{
			ShaderSpecification irradianceComputeShaderSpecification;
			irradianceComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/EnvironmentIrradiance_Compute.glsl";
			s_Data.IrradianceComputeShader = Shader::Create(irradianceComputeShaderSpecification);
			ComputePipelineSpecification irradianceComputePipelineSpecification;
			s_Data.IrradianceComputePipeline =
				ComputePipeline::Create(s_Data.IrradianceComputeShader, irradianceComputePipelineSpecification);
		}
	}

	void SceneRenderer::InitializeScene(SharedRef<Scene> scene)
	{
		NEO_CORE_ASSERT(scene);

		s_Data.SceneData = {};

		s_Data.ActiveScene = scene;

		CreateEnvironmentMap(scene->m_EnvironmentPath);

		ShaderSpecification skyboxShaderSpec;
		skyboxShaderSpec.ShaderPaths[ShaderType::Vertex] = "assets/shaders/Skybox_Vert.glsl";
		skyboxShaderSpec.ShaderPaths[ShaderType::Fragment] = "assets/shaders/Skybox_Frag.glsl";
		skyboxShaderSpec.VBLayout = std::vector<VertexBufferElement>{{ShaderDataType::Float2}};
		s_Data.SkyboxMaterial = Material(0, Shader::Create(skyboxShaderSpec));
		s_Data.SkyboxMaterial.SetTextureCube("u_Cubemap", s_Data.EnvFilteredTextureCube, 0);

		GraphicsPipelineSpecification skyboxGraphicsPipelineSpec;
		skyboxGraphicsPipelineSpec.Pass = s_Data.GeoPass;
		s_Data.SkyboxGraphicsPipeline =
			GraphicsPipeline::Create(s_Data.SkyboxMaterial.GetShader(), skyboxGraphicsPipelineSpec);
	}

	const SharedRef<Scene>& SceneRenderer::GetActiveScene()
	{
		return s_Data.ActiveScene;
	}

	void SceneRenderer::SetSelectedActor(SharedRef<Actor> actor)
	{
		s_Data.SelectedActor = actor;
	}

	SharedRef<Actor> SceneRenderer::GetSelectedActor()
	{
		return s_Data.SelectedActor;
	}

	void SceneRenderer::DestroyActor(SharedRef<Actor> actor)
	{
		NEO_CORE_ASSERT(s_Data.ActiveScene);

		s_Data.ActiveScene->DestroyActor(actor);
	}

	void SceneRenderer::SetViewportSize(uint32 width, uint32 height)
	{
		// TODO: Implement
	}

	void SceneRenderer::BeginScene(Camera* camera)
	{
		NEO_CORE_ASSERT(s_Data.ActiveScene, "Scene not initialized!");

		s_Data.SceneData.SceneCamera = camera;
	}

	void SceneRenderer::EndScene()
	{
		NEO_CORE_ASSERT(s_Data.ActiveScene, "");
		NEO_CORE_ASSERT(s_Data.SceneData.SceneCamera);

		FlushDrawList();
	}

	void SceneRenderer::SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform /*= glm::mat4(1.0f)*/,
								   bool wireframe /*=false*/)
	{
		s_Data.MeshDrawList.push_back({mesh, transform, wireframe});
	}

	void SceneRenderer::SubmitLight(const Light& light)
	{
		s_Data.Lights.push_back(light);
	}

	const SharedRef<RenderPass>& SceneRenderer::GetGeoPass()
	{
		return s_Data.GeoPass;
	}

	const SharedRef<TextureCube>& SceneRenderer::GetRadianceTex()
	{
		return s_Data.EnvFilteredTextureCube;
	}

	const SharedRef<TextureCube>& SceneRenderer::GetIrradianceTex()
	{
		return s_Data.IrradianceTextureCube;
	}

	const SharedRef<Texture2D>& SceneRenderer::GetBRDFLUTTex()
	{
		return s_Data.BRDFLUT;
	}

	void SceneRenderer::SetFocusPoint(const glm::vec2& point)
	{
		s_Data.FocusPoint = point;
	}

	void* SceneRenderer::GetFinalImageId()
	{
		return s_Data.PostProcessingPass->GetTargetFramebuffer()->GetSampledImageId();
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
		s_Data.EnvUnfilteredComputeShader->SetTexture2D("u_EquirectangularTex", 0, envMap, 0);

		s_Data.EnvUnfilteredTextureCube =
			TextureCube::Create({TextureUsageFlagBits::ShaderWrite | TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA16F,
								 TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, true, faceSize, faceSize});
		s_Data.EnvFilteredTextureCube =
			TextureCube::Create({TextureUsageFlagBits::ShaderWrite | TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA16F,
								 TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, true, faceSize, faceSize});

		s_Data.EnvUnfilteredComputeShader->SetStorageTextureCube("o_CubeMap", 0, s_Data.EnvUnfilteredTextureCube,
																		 0);
		Renderer::DispatchCompute(s_Data.EnvUnfilteredComputePipeline, faceSize / 32, faceSize / 32, 6);
		s_Data.EnvUnfilteredTextureCube->RegenerateMipMaps();

		s_Data.EnvUnfilteredComputeShader->SetStorageTextureCube("o_CubeMap", 0, s_Data.EnvFilteredTextureCube, 0);
		Renderer::DispatchCompute(s_Data.EnvUnfilteredComputePipeline, faceSize / 32, faceSize / 32, 6);
		s_Data.EnvFilteredTextureCube->RegenerateMipMaps();

		s_Data.EnvFilteredComputeShader->SetTextureCube("u_InputCubemap", 0, s_Data.EnvUnfilteredTextureCube, 0);
		for (uint32 level = 1, size = faceSize; level < s_Data.EnvUnfilteredTextureCube->GetMipLevelCount();
			 level++, size /= 2)
		{
			uint32 test = s_Data.EnvFilteredTextureCube->GetMipLevelCount();
			const uint32 numGroups = glm::max(1u, size / 32);
			struct
			{
				float MipCount;
				float MipLevel;
			} pc = {static_cast<float>(s_Data.EnvFilteredTextureCube->GetMipLevelCount()), static_cast<float>(level)};
			s_Data.EnvFilteredComputeShader->SetPushConstant("u_PushConstant", &pc);
			s_Data.EnvFilteredComputeShader->SetStorageTextureCube("o_OutputCubemap", 0,
																		   s_Data.EnvFilteredTextureCube, level);
			Renderer::DispatchCompute(s_Data.EnvFilteredComputePipeline, numGroups, numGroups, 6);
		}

		s_Data.IrradianceTextureCube = TextureCube::Create(
			{TextureUsageFlagBits::ShaderWrite | TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA16F, TextureWrap::Clamp,
			 TextureMinMagFilter::Linear, true, 1, true, irradianceMapSize, irradianceMapSize});
		s_Data.IrradianceComputeShader->SetTextureCube("u_InputCubemap", 0, s_Data.EnvFilteredTextureCube, 0);
		s_Data.IrradianceComputeShader->SetStorageTextureCube("o_OutputCubemap", 0, s_Data.IrradianceTextureCube,
																	  0);
		Renderer::DispatchCompute(s_Data.IrradianceComputePipeline, irradianceMapSize / 32, irradianceMapSize / 32, 6);
		s_Data.IrradianceTextureCube->RegenerateMipMaps();

		s_Data.BRDFLUT = Texture2D::Create("assets/textures/environment/BRDF_LUT.tga",
												   {TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA8, TextureWrap::Clamp});
	}

	void SceneRenderer::Shutdown()
	{
		s_Data = {};
	}

	void SceneRenderer::FlushDrawList()
	{
		GeometryPass();
		PostProcessingPass();
		s_Data.MeshDrawList.clear();
		s_Data.Lights.clear();
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
		for (auto& light : s_Data.Lights)
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
		cameraUBO.ViewProjection = sceneCamera->GetViewProjectionMatrix();
		cameraUBO.CameraPosition = glm::vec4(sceneCamera->GetPosition(), 1.f);

		// Render meshes
		for (auto& dc : s_Data.MeshDrawList)
		{
			cameraUBO.Model = dc.Transform;

			if (Renderer::IsWireframeEnabled() || dc.Wireframe)
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

			Renderer::SubmitMesh(dc.Mesh, dc.Transform, dc.Wireframe);
		}

		glm::mat4 viewRotation = sceneCamera->GetViewMatrix();
		viewRotation[3][0] = 0;
		viewRotation[3][1] = 0;
		viewRotation[3][2] = 0;
		glm::mat4 inverseVP = glm::inverse(sceneCamera->GetProjectionMatrix() * viewRotation);
		s_Data.SkyboxMaterial.GetShader()->SetUniformBuffer("CameraUBO", 0, &inverseVP);
		Renderer::SubmitFullscreenQuad(s_Data.SkyboxGraphicsPipeline);

		Renderer::EndRenderPass();
	}

	void SceneRenderer::PostProcessingPass()
	{
		Renderer::BeginRenderPass(s_Data.PostProcessingPass);
		s_Data.PostProcessingShader->SetTexture2D("u_Texture", 0,
														  s_Data.GeoPass->GetTargetFramebuffer()->GetSampledImage(), 0);
		Renderer::SubmitFullscreenQuad(s_Data.PostProcessingPipeline);
		Renderer::EndRenderPass();
	}

} // namespace Neon
