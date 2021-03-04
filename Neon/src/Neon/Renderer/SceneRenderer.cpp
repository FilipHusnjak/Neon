#include "neopch.h"

#include "Neon/Renderer/Framebuffer.h"
#include "Neon/Renderer/Renderer.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Components.h"

namespace Neon
{
	struct SceneRendererData
	{
		SharedRef<Scene> ActiveScene = nullptr;
		Entity SelectedEntity = {};

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

		SharedRef<Shader> InitialSpectrumShader;
		SharedRef<ComputePipeline> InitialSpectrumPipeline;

		SharedRef<Texture2D> H0k;
		SharedRef<Texture2D> H0minusk;

		SharedRef<Texture2D> Noise0;
		SharedRef<Texture2D> Noise1;
		SharedRef<Texture2D> Noise2;
		SharedRef<Texture2D> Noise3;

		SharedRef<Shader> CurrentSpectrumShader;
		SharedRef<ComputePipeline> CurrentSpectrumPipeline;

		SharedRef<Texture2D> HktDy;
		SharedRef<Texture2D> HktDx;
		SharedRef<Texture2D> HktDz;

		SharedRef<Shader> TwiddleIndicesShader;
		SharedRef<ComputePipeline> TwiddleIndicesPipeline;

		SharedRef<Texture2D> TwiddleIndices;

		SharedRef<Shader> ButterflyShader;
		SharedRef<ComputePipeline> ButterflyPipeline;

		SharedRef<Texture2D> PingPong;

		SharedRef<Shader> DisplacementShader;
		SharedRef<ComputePipeline> DisplacementPipeline;

		SharedRef<Texture2D> DisplacementY;

		float CurrentTime = 0;
	};

	static SceneRendererData s_Data;

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

		{
			ShaderSpecification initialSpectrumComputeShaderSpecification;
			initialSpectrumComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/fft/InitialSpectrum_Compute.glsl";
			s_Data.InitialSpectrumShader = Shader::Create(initialSpectrumComputeShaderSpecification);
			ComputePipelineSpecification initialSpectrumComputePipelineSpecification;
			s_Data.InitialSpectrumPipeline =
				ComputePipeline::Create(s_Data.InitialSpectrumShader, initialSpectrumComputePipelineSpecification);
		}

		{
			ShaderSpecification currentSpectrumComputeShaderSpecification;
			currentSpectrumComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/fft/CurrentSpectrum_Compute.glsl";
			s_Data.CurrentSpectrumShader = Shader::Create(currentSpectrumComputeShaderSpecification);
			ComputePipelineSpecification currentSpectrumComputePipelineSpecification;
			s_Data.CurrentSpectrumPipeline =
				ComputePipeline::Create(s_Data.CurrentSpectrumShader, currentSpectrumComputePipelineSpecification);
		}

		{
			ShaderSpecification twiddleIndicesComputeShaderSpecification;
			twiddleIndicesComputeShaderSpecification.ShaderVariableCounts["BitReversedUBO"] = 256;
			twiddleIndicesComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/fft/TwiddleIndices_Compute.glsl";
			s_Data.TwiddleIndicesShader = Shader::Create(twiddleIndicesComputeShaderSpecification);
			ComputePipelineSpecification twiddleIndicesComputePipelineSpecification;
			s_Data.TwiddleIndicesPipeline =
				ComputePipeline::Create(s_Data.TwiddleIndicesShader, twiddleIndicesComputePipelineSpecification);
		}

		{
			ShaderSpecification butterflyComputeShaderSpecification;
			butterflyComputeShaderSpecification.ShaderPaths[ShaderType::Compute] = "assets/shaders/fft/Butterfly_Compute.glsl";
			butterflyComputeShaderSpecification.ShaderVariableCounts["u_PingPong"] = 2;
			s_Data.ButterflyShader = Shader::Create(butterflyComputeShaderSpecification);
			ComputePipelineSpecification butterflyComputePipelineSpecification;
			s_Data.ButterflyPipeline = ComputePipeline::Create(s_Data.ButterflyShader, butterflyComputePipelineSpecification);
		}

		{
			ShaderSpecification displacementComputeShaderSpecification;
			displacementComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/fft/Displacement_Compute.glsl";
			displacementComputeShaderSpecification.ShaderVariableCounts["u_PingPong"] = 2;
			s_Data.DisplacementShader = Shader::Create(displacementComputeShaderSpecification);
			ComputePipelineSpecification displacementComputePipelineSpecification;
			s_Data.DisplacementPipeline =
				ComputePipeline::Create(s_Data.DisplacementShader, displacementComputePipelineSpecification);
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
		s_Data.SkyboxGraphicsPipeline = GraphicsPipeline::Create(s_Data.SkyboxMaterial.GetShader(), skyboxGraphicsPipelineSpec);
	}

	const SharedRef<Scene>& SceneRenderer::GetActiveScene()
	{
		return s_Data.ActiveScene;
	}

	void SceneRenderer::SetSelectedEntity(Entity entity)
	{
		s_Data.SelectedEntity = entity;
	}

	Entity SceneRenderer::GetSelectedEntity()
	{
		return s_Data.SelectedEntity;
	}

	Entity SceneRenderer::CreateEntity(const std::string& name /*= "Entity"*/)
	{
		NEO_CORE_ASSERT(s_Data.ActiveScene);

		return s_Data.ActiveScene->CreateEntity(name);
	}

	void SceneRenderer::DestroyEntity(Entity entity)
	{
		NEO_CORE_ASSERT(s_Data.ActiveScene);

		s_Data.ActiveScene->DestroyEntity(entity);
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

	void SceneRenderer::SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform /*= glm::mat4(1.0f)*/)
	{
		s_Data.MeshDrawList.push_back({mesh, transform});
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
		//return s_Data.PostProcessingPass->GetTargetFramebuffer()->GetSampledImageId();
		return s_Data.DisplacementY->GetRendererId();
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

		s_Data.EnvUnfilteredComputeShader->SetStorageTextureCube("o_CubeMap", 0, s_Data.EnvUnfilteredTextureCube, 0);
		Renderer::DispatchCompute(s_Data.EnvUnfilteredComputePipeline, faceSize / 32, faceSize / 32, 6);
		s_Data.EnvUnfilteredTextureCube->RegenerateMipMaps();

		s_Data.EnvUnfilteredComputeShader->SetStorageTextureCube("o_CubeMap", 0, s_Data.EnvFilteredTextureCube, 0);
		Renderer::DispatchCompute(s_Data.EnvUnfilteredComputePipeline, faceSize / 32, faceSize / 32, 6);
		s_Data.EnvFilteredTextureCube->RegenerateMipMaps();

		s_Data.EnvFilteredComputeShader->SetTextureCube("u_InputCubemap", 0, s_Data.EnvUnfilteredTextureCube, 0);
		for (uint32 level = 1, size = faceSize; level < s_Data.EnvUnfilteredTextureCube->GetMipLevelCount(); level++, size /= 2)
		{
			uint32 test = s_Data.EnvFilteredTextureCube->GetMipLevelCount();
			const uint32 numGroups = glm::max(1u, size / 32);
			struct
			{
				float MipCount;
				float MipLevel;
			} pc = {static_cast<float>(s_Data.EnvFilteredTextureCube->GetMipLevelCount()), static_cast<float>(level)};
			s_Data.EnvFilteredComputeShader->SetPushConstant("u_PushConstant", &pc);
			s_Data.EnvFilteredComputeShader->SetStorageTextureCube("o_OutputCubemap", 0, s_Data.EnvFilteredTextureCube, level);
			Renderer::DispatchCompute(s_Data.EnvFilteredComputePipeline, numGroups, numGroups, 6);
		}

		s_Data.IrradianceTextureCube = TextureCube::Create({TextureUsageFlagBits::ShaderWrite | TextureUsageFlagBits::ShaderRead,
															TextureFormat::RGBA16F, TextureWrap::Clamp, TextureMinMagFilter::Linear,
															true, 1, true, irradianceMapSize, irradianceMapSize});
		s_Data.IrradianceComputeShader->SetTextureCube("u_InputCubemap", 0, s_Data.EnvFilteredTextureCube, 0);
		s_Data.IrradianceComputeShader->SetStorageTextureCube("o_OutputCubemap", 0, s_Data.IrradianceTextureCube, 0);
		Renderer::DispatchCompute(s_Data.IrradianceComputePipeline, irradianceMapSize / 32, irradianceMapSize / 32, 6);
		s_Data.IrradianceTextureCube->RegenerateMipMaps();

		s_Data.BRDFLUT = Texture2D::Create("assets/textures/environment/BRDF_LUT.tga",
										   {TextureUsageFlagBits::ShaderRead, TextureFormat::RGBA8, TextureWrap::Clamp});

		s_Data.H0k =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, 256, 256});
		s_Data.H0minusk =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, 256, 256});
		s_Data.HktDy =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, 256, 256});
		s_Data.HktDx =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, 256, 256});
		s_Data.HktDz =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, 256, 256});
		s_Data.Noise0 = Texture2D::Create("assets/textures/fft/Noise256_0.jpg",
										  {TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite,
										   TextureFormat::RGBA8, TextureWrap::Clamp, TextureMinMagFilter::Nearest});
		s_Data.Noise1 = Texture2D::Create("assets/textures/fft/Noise256_1.jpg",
										  {TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite,
										   TextureFormat::RGBA8, TextureWrap::Clamp, TextureMinMagFilter::Nearest});
		s_Data.Noise2 = Texture2D::Create("assets/textures/fft/Noise256_2.jpg",
										  {TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite,
										   TextureFormat::RGBA8, TextureWrap::Clamp, TextureMinMagFilter::Nearest});
		s_Data.Noise3 = Texture2D::Create("assets/textures/fft/Noise256_3.jpg",
										  {TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite,
										   TextureFormat::RGBA8, TextureWrap::Clamp, TextureMinMagFilter::Nearest});

		s_Data.InitialSpectrumShader->SetStorageTexture2D("u_H0k", 0, s_Data.H0k, 0);
		s_Data.InitialSpectrumShader->SetStorageTexture2D("u_H0minusk", 0, s_Data.H0minusk, 0);

		s_Data.InitialSpectrumShader->SetTexture2D("u_Noise0", 0, s_Data.Noise0, 0);
		s_Data.InitialSpectrumShader->SetTexture2D("u_Noise1", 0, s_Data.Noise1, 0);
		s_Data.InitialSpectrumShader->SetTexture2D("u_Noise2", 0, s_Data.Noise2, 0);
		s_Data.InitialSpectrumShader->SetTexture2D("u_Noise3", 0, s_Data.Noise3, 0);

		Renderer::DispatchCompute(s_Data.InitialSpectrumPipeline, 256 / 32, 256 / 32, 1);

		s_Data.CurrentSpectrumShader->SetStorageTexture2D("u_HktDy", 0, s_Data.HktDy, 0);
		s_Data.CurrentSpectrumShader->SetStorageTexture2D("u_HktDx", 0, s_Data.HktDx, 0);
		s_Data.CurrentSpectrumShader->SetStorageTexture2D("u_HktDz", 0, s_Data.HktDz, 0);

		s_Data.CurrentSpectrumShader->SetStorageTexture2D("u_H0k", 0, s_Data.H0k, 0);
		s_Data.CurrentSpectrumShader->SetStorageTexture2D("u_H0minusk", 0, s_Data.H0minusk, 0);

		int bitReversedIndices[256];
		for (int i = 0; i < 256; i++)
		{
			bitReversedIndices[i] = 0;
			int n = i;
			int power = 7;
			while (n != 0)
			{
				bitReversedIndices[i] += (n & 1) << power;
				n = n >> 1;
				power -= 1;
			}
		}

		s_Data.TwiddleIndices =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, 8, 256});

		s_Data.TwiddleIndicesShader->SetStorageTexture2D("u_TwiddleIndices", 0, s_Data.TwiddleIndices, 0);
		s_Data.TwiddleIndicesShader->SetStorageBuffer("BitReversedUBO", bitReversedIndices, sizeof(bitReversedIndices));
		Renderer::DispatchCompute(s_Data.TwiddleIndicesPipeline, 8, 256 / 32, 1);

		s_Data.PingPong =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, 256, 256});
		s_Data.ButterflyShader->SetStorageTexture2D("u_TwiddleIndices", 0, s_Data.TwiddleIndices, 0);
		s_Data.ButterflyShader->SetStorageTexture2D("u_PingPong", 0, s_Data.HktDy, 0);
		s_Data.ButterflyShader->SetStorageTexture2D("u_PingPong", 1, s_Data.PingPong, 0);

		s_Data.DisplacementY =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, 256, 256});
		s_Data.DisplacementShader->SetStorageTexture2D("u_Displacement", 0, s_Data.DisplacementY, 0);
		s_Data.DisplacementShader->SetStorageTexture2D("u_PingPong", 0, s_Data.HktDy, 0);
		s_Data.DisplacementShader->SetStorageTexture2D("u_PingPong", 1, s_Data.PingPong, 0);
	}

	void SceneRenderer::Shutdown()
	{
		s_Data = {};
	}

	void SceneRenderer::FlushDrawList()
	{
		s_Data.CurrentSpectrumShader->SetUniformBuffer("TimeUBO", 0, &s_Data.CurrentTime, sizeof(s_Data.CurrentTime));
		Renderer::DispatchCompute(s_Data.CurrentSpectrumPipeline, 256 / 32, 256 / 32, 1);
		s_Data.CurrentTime += 0.01f;

		int pingPong = 0;
		// IFFT horizontal
		for (int i = 0; i < 8; i++)
		{
			struct
			{
				int Stage;
				int PingPong;
				int Direction;
			} butterflyData = {i, pingPong, 0};

			s_Data.ButterflyShader->SetUniformBuffer("UBO", 0, &butterflyData, sizeof(butterflyData));
			Renderer::DispatchCompute(s_Data.ButterflyPipeline, 256 / 32, 256 / 32, 1);

			pingPong++;
			pingPong %= 2;
		}

		// IFFT vertical
		for (int i = 0; i < 8; i++)
		{
			struct
			{
				int Stage;
				int PingPong;
				int Direction;
			} butterflyData = {i, pingPong, 1};

			s_Data.ButterflyShader->SetUniformBuffer("UBO", 0, &butterflyData, sizeof(butterflyData));
			Renderer::DispatchCompute(s_Data.ButterflyPipeline, 256 / 32, 256 / 32, 1);

			pingPong++;
			pingPong %= 2;
		}

		s_Data.DisplacementShader->SetUniformBuffer("UBO", 0, &pingPong, sizeof(pingPong));
		Renderer::DispatchCompute(s_Data.DisplacementPipeline, 256 / 32, 256 / 32, 1);

		GeometryPass();
		PostProcessingPass();
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
		s_Data.SkyboxMaterial.GetShader()->SetUniformBuffer("CameraUBO", 0, &inverseVP);
		Renderer::SubmitFullscreenQuad(s_Data.SkyboxGraphicsPipeline);

		Renderer::EndRenderPass();
	}

	void SceneRenderer::PostProcessingPass()
	{
		Renderer::BeginRenderPass(s_Data.PostProcessingPass);
		s_Data.PostProcessingShader->SetTexture2D("u_Texture", 0, s_Data.GeoPass->GetTargetFramebuffer()->GetSampledImage(), 0);
		Renderer::SubmitFullscreenQuad(s_Data.PostProcessingPipeline);
		Renderer::EndRenderPass();
	}

} // namespace Neon
