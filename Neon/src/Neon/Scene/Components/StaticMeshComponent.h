#pragma once

#include "Neon/Renderer/StaticMesh.h"
#include "Neon/Scene/Components/PrimitiveComponent.h"

namespace Neon
{
	class StaticMeshComponent : public PrimitiveComponent
	{
	public:
		StaticMeshComponent() = default;
		StaticMeshComponent(Actor* owner, const SharedRef<StaticMesh>& staticMesh);
		StaticMeshComponent(const StaticMeshComponent& other) = default;
		virtual ~StaticMeshComponent();

		void LoadMesh(const std::string& filename);

		SharedRef<StaticMesh> GetMesh() const
		{
			return m_StaticMesh;
		}

	private:
		SharedRef<StaticMesh> m_StaticMesh;
	};
} // namespace Neon
