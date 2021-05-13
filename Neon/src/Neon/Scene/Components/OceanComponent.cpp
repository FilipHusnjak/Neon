#include "neopch.h"

#include "Neon/Renderer/Renderer.h"
#include "Neon/Renderer/RendererContext.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Components/OceanComponent.h"

namespace Neon
{
	OceanComponent::OceanComponent(uint32 n)
		: m_N(n)
	{
		m_LogN = static_cast<uint32>(std::log2(n));
		{
			ShaderSpecification initialSpectrumComputeShaderSpecification;
			initialSpectrumComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/fft/InitialSpectrum_Compute.glsl";
			m_InitialSpectrumShader = Shader::Create(initialSpectrumComputeShaderSpecification);
			ComputePipelineSpecification initialSpectrumComputePipelineSpecification;
			m_InitialSpectrumPipeline =
				ComputePipeline::Create(m_InitialSpectrumShader, initialSpectrumComputePipelineSpecification);
		}

		{
			ShaderSpecification currentSpectrumComputeShaderSpecification;
			currentSpectrumComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/fft/CurrentSpectrum_Compute.glsl";
			m_CurrentSpectrumShader = Shader::Create(currentSpectrumComputeShaderSpecification);
			ComputePipelineSpecification currentSpectrumComputePipelineSpecification;
			m_CurrentSpectrumPipeline =
				ComputePipeline::Create(m_CurrentSpectrumShader, currentSpectrumComputePipelineSpecification);
		}

		{
			ShaderSpecification twiddleFactorsComputeShaderSpecification;
			twiddleFactorsComputeShaderSpecification.ShaderVariableCounts["BitReversedUBO"] = m_N;
			twiddleFactorsComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/fft/TwiddleFactors_Compute.glsl";
			m_TwiddleFactorsShader = Shader::Create(twiddleFactorsComputeShaderSpecification);
			ComputePipelineSpecification twiddleFactorsComputePipelineSpecification;
			m_TwiddleFactorsPipeline = ComputePipeline::Create(m_TwiddleFactorsShader, twiddleFactorsComputePipelineSpecification);
		}

		{
			ShaderSpecification butterflyComputeShaderSpecification;
			butterflyComputeShaderSpecification.ShaderPaths[ShaderType::Compute] = "assets/shaders/fft/Butterfly_Compute.glsl";
			butterflyComputeShaderSpecification.ShaderVariableCounts["u_PingPong"] = 6;
			m_ButterflyShader = Shader::Create(butterflyComputeShaderSpecification);
			ComputePipelineSpecification butterflyComputePipelineSpecification;
			m_ButterflyPipeline = ComputePipeline::Create(m_ButterflyShader, butterflyComputePipelineSpecification);
		}

		{
			ShaderSpecification displacementComputeShaderSpecification;
			displacementComputeShaderSpecification.ShaderPaths[ShaderType::Compute] =
				"assets/shaders/fft/Displacement_Compute.glsl";
			displacementComputeShaderSpecification.ShaderVariableCounts["u_ButterflyResult"] = 3;
			displacementComputeShaderSpecification.ShaderVariableCounts["u_Result"] = 3;
			m_DisplacementShader = Shader::Create(displacementComputeShaderSpecification);
			ComputePipelineSpecification displacementComputePipelineSpecification;
			m_DisplacementPipeline = ComputePipeline::Create(m_DisplacementShader, displacementComputePipelineSpecification);
		}

		{
			ShaderSpecification gradientsComputeShaderSpecification;
			gradientsComputeShaderSpecification.ShaderPaths[ShaderType::Compute] = "assets/shaders/fft/Gradients_Compute.glsl";
			m_GradientsShader = Shader::Create(gradientsComputeShaderSpecification);
			ComputePipelineSpecification gradientsComputePipelineSpecification;
			m_GradientsPipeline = ComputePipeline::Create(m_GradientsShader, gradientsComputePipelineSpecification);
		}

		float L = 20.f;

		m_H0k = Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
								   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, m_N, m_N});
		m_HktDy = Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
									 TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});
		m_HktDx = Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
									 TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});
		m_HktDz = Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
									 TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});
		m_Noise0 = Texture2D::Create("assets/textures/fft/Noise512_0.jpg",
									 {TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA8,
									  TextureWrap::Clamp, TextureMinMagFilter::Nearest});
		m_Noise1 = Texture2D::Create("assets/textures/fft/Noise512_1.jpg",
									 {TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA8,
									  TextureWrap::Clamp, TextureMinMagFilter::Nearest});
		m_Noise2 = Texture2D::Create("assets/textures/fft/Noise512_2.jpg",
									 {TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA8,
									  TextureWrap::Clamp, TextureMinMagFilter::Nearest});
		m_Noise3 = Texture2D::Create("assets/textures/fft/Noise256_3.jpg",
									 {TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA8,
									  TextureWrap::Clamp, TextureMinMagFilter::Nearest});

		m_InitialSpectrumShader->SetStorageTexture2D("u_H0k", 0, m_H0k, 0);

		m_InitialSpectrumShader->SetTexture2D("u_Noise0", 0, m_Noise0, 0);
		m_InitialSpectrumShader->SetTexture2D("u_Noise1", 0, m_Noise1, 0);
		m_InitialSpectrumShader->SetTexture2D("u_Noise2", 0, m_Noise2, 0);
		m_InitialSpectrumShader->SetTexture2D("u_Noise3", 0, m_Noise3, 0);

		struct
		{
			uint32 N;
			float L;
			float A;
			float Windspeed;
			glm::vec2 WindDir;
		} properties = {m_N, L, 0.45f * 1e-3f, 6.5f, glm::vec2{-0.4f, -0.9f}};
		m_InitialSpectrumShader->SetUniformBuffer("PropertiesUBO", 0, &properties);

		m_GradientsShader->SetUniformBuffer("PropertiesUBO", 0, &properties);

		Renderer::DispatchCompute(m_InitialSpectrumPipeline, m_N / 32, m_N / 32, 1);

		m_CurrentSpectrumShader->SetStorageTexture2D("u_HktDy", 0, m_HktDy, 0);
		m_CurrentSpectrumShader->SetStorageTexture2D("u_HktDx", 0, m_HktDx, 0);
		m_CurrentSpectrumShader->SetStorageTexture2D("u_HktDz", 0, m_HktDz, 0);

		m_CurrentSpectrumShader->SetStorageTexture2D("u_H0k", 0, m_H0k, 0);

		m_CurrentSpectrumShader->SetUniformBuffer("PropertiesUBO", 0, &properties);

		std::vector<uint32> bitsReversed;
		bitsReversed.resize(m_N);
		for (uint32 i = 0; i < m_N; i++)
		{
			bitsReversed[i] = 0;
			uint32 n = i;
			uint32 power = m_LogN - 1;
			while (n != 0)
			{
				bitsReversed[i] += (n & 1) << power;
				n = n >> 1;
				power -= 1;
			}
		}

		m_TwiddleFactors =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Nearest, true, 1, false, m_LogN, m_N});

		m_TwiddleFactorsShader->SetStorageTexture2D("u_TwiddleFactors", 0, m_TwiddleFactors, 0);
		m_TwiddleFactorsShader->SetStorageBuffer("BitReversedUBO", bitsReversed.data(),
												 static_cast<uint32>(bitsReversed.size() * sizeof(bitsReversed[0])));
		m_TwiddleFactorsShader->SetUniformBuffer("PropertiesUBO", 0, &properties);
		Renderer::DispatchCompute(m_TwiddleFactorsPipeline, m_LogN, m_N / 32, 1);

		m_PingPong[0] =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});
		m_PingPong[1] =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});
		m_PingPong[2] =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});
		m_PingPong[3] =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});
		m_PingPong[4] =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Clamp, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});

		m_ButterflyShader->SetStorageTexture2D("m_TwiddleFactors", 0, m_TwiddleFactors, 0);
		m_ButterflyShader->SetStorageTexture2D("u_PingPong", 0, m_HktDy, 0);
		m_ButterflyShader->SetStorageTexture2D("u_PingPong", 1, m_PingPong[0], 0);
		m_ButterflyShader->SetStorageTexture2D("u_PingPong", 2, m_HktDx, 0);
		m_ButterflyShader->SetStorageTexture2D("u_PingPong", 3, m_PingPong[1], 0);
		m_ButterflyShader->SetStorageTexture2D("u_PingPong", 4, m_HktDz, 0);
		m_ButterflyShader->SetStorageTexture2D("u_PingPong", 5, m_PingPong[2], 0);

		m_Displacement =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Repeat, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});

		m_Gradients =
			Texture2D::Create({TextureUsageFlagBits::ShaderRead | TextureUsageFlagBits::ShaderWrite, TextureFormat::RGBA32F,
							   TextureWrap::Repeat, TextureMinMagFilter::Linear, true, 1, false, m_N, m_N});

		m_DisplacementShader->SetStorageTexture2D("u_Displacement", 0, m_Displacement, 0);

		m_DisplacementShader->SetStorageTexture2D("u_ButterflyResult", 0, m_HktDy, 0);
		m_DisplacementShader->SetStorageTexture2D("u_ButterflyResult", 1, m_HktDx, 0);
		m_DisplacementShader->SetStorageTexture2D("u_ButterflyResult", 2, m_HktDz, 0);

		m_DisplacementShader->SetUniformBuffer("PropertiesUBO", 0, &properties);

		m_GradientsShader->SetStorageTexture2D("u_Gradients", 0, m_Gradients, 0);
		m_GradientsShader->SetStorageTexture2D("u_Displacement", 0, m_Displacement, 0);

		ShaderSpecification oceanShaderSpecification;
		oceanShaderSpecification.ShaderPaths[ShaderType::Vertex] = "assets/shaders/Ocean_Vert.glsl";
		oceanShaderSpecification.ShaderPaths[ShaderType::Fragment] = "assets/shaders/Ocean_Frag.glsl";
		oceanShaderSpecification.VBLayout = std::vector<VertexBufferElement>{{ShaderDataType::Float2}};
		GraphicsPipelineSpecification oceanPipelineSpecification;
		oceanPipelineSpecification.Pass = SceneRenderer::GetGeoPass();
		oceanPipelineSpecification.Mode = PolygonMode::Fill;

		m_Mesh = Mesh::GenerateGridMesh(1000, 1000, oceanShaderSpecification, oceanPipelineSpecification);

		SharedRef<Shader> oceanShader = m_Mesh->GetShader();
		oceanShader->SetUniformBuffer("PropertiesUBO", 0, &L);
		oceanShader->SetTexture2D("u_Displacement", 0, m_Displacement, 0);
		oceanShader->SetTexture2D("u_Gradients", 0, m_Gradients, 0);
		oceanShader->SetTextureCube("u_EnvRadianceTex", 0, SceneRenderer::GetRadianceTex(), 0);

		oceanShader = m_Mesh->GetWireframeShader();
		oceanShader->SetUniformBuffer("PropertiesUBO", 0, &L);
		oceanShader->SetTexture2D("u_Displacement", 0, m_Displacement, 0);
	}

	OceanComponent::~OceanComponent()
	{
		RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(m_Mesh));
	}

	void OceanComponent::OnUpdate(float deltaSeconds)
	{
		m_CurrentSpectrumShader->SetUniformBuffer("TimeUBO", 0, &m_CurrentTimeSeconds);
		Renderer::DispatchCompute(m_CurrentSpectrumPipeline, m_N / 32, m_N / 32, 1);
		m_CurrentTimeSeconds += deltaSeconds;

		uint32 pingPong = 0;
		for (uint32 direction = 0; direction < 2; direction++)
		{
			for (uint32 i = 0; i < m_LogN; i++)
			{
				struct
				{
					uint32 Stage;
					uint32 PingPong;
					uint32 Direction;
				} butterflyData = {i, pingPong, direction};

				m_ButterflyShader->SetUniformBuffer("PropertiesUBO", 0, &butterflyData);
				Renderer::DispatchCompute(m_ButterflyPipeline, m_N / 32, m_N / 32, 1);

				pingPong++;
				pingPong %= 2;
			}
		}

		Renderer::DispatchCompute(m_DisplacementPipeline, m_N / 32, m_N / 32, 1);

		Renderer::DispatchCompute(m_GradientsPipeline, m_N / 32, m_N / 32, 1);
	}

} // namespace Neon