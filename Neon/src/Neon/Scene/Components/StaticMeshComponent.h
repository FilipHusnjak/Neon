#pragma once

#include "Neon/Renderer/StaticMesh.h"
#include "Neon/Scene/Components/MeshComponent.h"

namespace Neon
{
	class StaticMeshComponent : public MeshComponent
	{
	public:
		StaticMeshComponent() = default;
		StaticMeshComponent(const StaticMeshComponent& other) = default;
		StaticMeshComponent(const SharedRef<StaticMesh>& staticMesh);
		~StaticMeshComponent();

		void LoadMesh(const std::string& filename);

		SharedRef<StaticMesh> GetMesh() const
		{
			return m_StaticMesh;
		}

	private:
		SharedRef<StaticMesh> m_StaticMesh;
	};
} // namespace Neon
