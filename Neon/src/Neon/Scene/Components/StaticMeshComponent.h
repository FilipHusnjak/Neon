#pragma once

#include "Neon/Renderer/StaticMesh.h"
#include "Neon/Scene/Components/PrimitiveComponent.h"

namespace Neon
{
	class StaticMeshComponent : public PrimitiveComponent
	{
	public:
		StaticMeshComponent(Actor* owner, const SharedRef<StaticMesh>& staticMesh = nullptr);
		StaticMeshComponent(const StaticMeshComponent& other) = default;
		virtual ~StaticMeshComponent();

		virtual void TickComponent(float deltaSeconds) override;

		virtual void LoadMesh(const std::string& filename) override;

		virtual SharedRef<Mesh> GetMesh() const override
		{
			return m_StaticMesh;
		}

	private:
		SharedRef<StaticMesh> m_StaticMesh;
	};
} // namespace Neon
