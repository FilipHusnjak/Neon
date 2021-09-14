#pragma once

#include "Neon/Renderer/StaticMesh.h"

namespace Neon
{
	class MeshFactory
	{
	public:
		static SharedRef<StaticMesh> CreateGrid(uint32 countW, uint32 countH, ShaderSpecification& shaderSpec,
										   GraphicsPipelineSpecification& pipelineSpec);
		static SharedRef<StaticMesh> CreateBox(const glm::vec3& size);
		static SharedRef<StaticMesh> CreateSphere(float radius);
		static SharedRef<StaticMesh> CreateCapsule(float radius, float height);
	};
} // namespace Neon
