#pragma once

#include "Neon/Renderer/Mesh.h"

namespace Neon
{
	class OceanComponent
	{
	public:
		OceanComponent(uint32 n);
		OceanComponent(const OceanComponent& other) = default;
		~OceanComponent();

		void OnUpdate(float deltaSeconds);

		operator SharedRef<Mesh>()
		{
			return m_Mesh;
		}

	private:
		float m_CurrentTimeSeconds = 0.f;

		uint32 m_N;
		uint32 m_LogN;

		SharedRef<Shader> m_InitialSpectrumShader;
		SharedRef<ComputePipeline> m_InitialSpectrumPipeline;
		SharedRef<Texture2D> m_H0k;
		SharedRef<Texture2D> m_Noise0;
		SharedRef<Texture2D> m_Noise1;
		SharedRef<Texture2D> m_Noise2;
		SharedRef<Texture2D> m_Noise3;

		SharedRef<Shader> m_CurrentSpectrumShader;
		SharedRef<ComputePipeline> m_CurrentSpectrumPipeline;
		SharedRef<Texture2D> m_HktDy;
		SharedRef<Texture2D> m_HktDx;
		SharedRef<Texture2D> m_HktDz;

		SharedRef<Shader> m_TwiddleFactorsShader;
		SharedRef<ComputePipeline> m_TwiddleFactorsPipeline;
		SharedRef<Texture2D> m_TwiddleFactors;

		SharedRef<Shader> m_ButterflyShader;
		SharedRef<ComputePipeline> m_ButterflyPipeline;
		SharedRef<Texture2D> m_PingPong[5];

		SharedRef<Shader> m_DisplacementShader;
		SharedRef<ComputePipeline> m_DisplacementPipeline;
		SharedRef<Texture2D> m_Displacement;

		SharedRef<Shader> m_GradientsShader;
		SharedRef<ComputePipeline> m_GradientsPipeline;
		SharedRef<Texture2D> m_Gradients;

		SharedRef<Mesh> m_Mesh;
	};
} // namespace Neon
