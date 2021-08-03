#pragma once

#include "Neon/Renderer/SkeletalMesh.h"
#include "Neon/Scene/Components/PrimitiveComponent.h"

namespace Neon
{
	class SkeletalMeshComponent : public PrimitiveComponent
	{
	public:
		SkeletalMeshComponent() = default;
		SkeletalMeshComponent(Actor* owner, const SharedRef<SkeletalMesh>& skeletalMesh);
		SkeletalMeshComponent(const SkeletalMeshComponent& other) = default;
		virtual ~SkeletalMeshComponent();

		virtual void CreatePhysicsBody(const std::string& boneName = std::string()) override;

		virtual void TickComponent(float deltaSeconds) override;

		void LoadMesh(const std::string& filename);

		SharedRef<SkeletalMesh> GetMesh() const
		{
			return m_SkeletalMesh;
		}

	private:
		SharedRef<SkeletalMesh> m_SkeletalMesh;

		std::map<std::string, SharedRef<PhysicsBody>> m_PhysicsBodyMap;
	};
} // namespace Neon
