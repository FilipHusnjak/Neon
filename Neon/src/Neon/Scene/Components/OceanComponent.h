#pragma once

#include "Neon/Renderer/Mesh.h"
#include "Neon/Scene/Components/ActorComponent.h"

namespace Neon
{
	class OceanComponent : public ActorComponent
	{
	public:
		OceanComponent(uint32 n);
		OceanComponent(const OceanComponent& other) = default;
		virtual ~OceanComponent();

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

		SharedRef<Texture2D> m_Foam;

		SharedRef<Shader> m_CurrentSpectrumShader;
		SharedRef<ComputePipeline> m_CurrentSpectrumPipeline;
		SharedRef<Texture2D> m_HktDy;
		SharedRef<Texture2D> m_HktDx;
		SharedRef<Texture2D> m_HktDz;
		SharedRef<Texture2D> m_HktDyDx;
		SharedRef<Texture2D> m_HktDyDz;
		SharedRef<Texture2D> m_HktDxDx;
		SharedRef<Texture2D> m_HktDzDz;
		SharedRef<Texture2D> m_HktDxDz;

		SharedRef<Shader> m_TwiddleFactorsShader;
		SharedRef<ComputePipeline> m_TwiddleFactorsPipeline;
		SharedRef<Texture2D> m_TwiddleFactors;

		SharedRef<Shader> m_ButterflyShader;
		SharedRef<ComputePipeline> m_ButterflyPipeline;
		SharedRef<Texture2D> m_PingPong[8];

		SharedRef<Shader> m_DisplacementShader;
		SharedRef<ComputePipeline> m_DisplacementPipeline;
		SharedRef<Texture2D> m_DisplacementY;
		SharedRef<Texture2D> m_DisplacementX;
		SharedRef<Texture2D> m_DisplacementZ;
		SharedRef<Texture2D> m_DerivativeYX;
		SharedRef<Texture2D> m_DerivativeYZ;
		SharedRef<Texture2D> m_DerivativeXX;
		SharedRef<Texture2D> m_DerivativeZZ;
		SharedRef<Texture2D> m_DerivativeXZ;

		SharedRef<Shader> m_JacobianShader;
		SharedRef<ComputePipeline> m_JacobianPipeline;
		SharedRef<Texture2D> m_JacobianTexture;

		SharedRef<Mesh> m_Mesh;
	};
} // namespace Neon
