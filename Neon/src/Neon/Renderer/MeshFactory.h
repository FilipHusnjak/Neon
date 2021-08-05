#pragma once

#include "Neon/Renderer/StaticMesh.h"

namespace Neon
{
	class MeshFactory
	{
	public:
		static SharedRef<StaticMesh> CreateBox(const glm::vec3& size);
		static SharedRef<StaticMesh> CreateSphere(float radius);
		static SharedRef<StaticMesh> CreateCapsule(float radius, float height);
	};
} // namespace Neon
