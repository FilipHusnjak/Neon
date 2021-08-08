#pragma once

#include "Neon/Renderer/SkeletalMesh.h"
#include "Neon/Scene/Components/PrimitiveComponent.h"

namespace Neon
{
	class SkeletalMeshComponent : public PrimitiveComponent
	{
	public:
		SkeletalMeshComponent(Actor* owner, const SharedRef<SkeletalMesh>& skeletalMesh = nullptr);
		SkeletalMeshComponent(const SkeletalMeshComponent& other) = default;
		virtual ~SkeletalMeshComponent();

		virtual void CreatePhysicsBody(PhysicsBodyType bodyType, const std::string& boneName = std::string(),
									   const SharedRef<PhysicsMaterial>& material = nullptr) override;

		virtual void TickComponent(float deltaSeconds) override;

		virtual void LoadMesh(const std::string& filename) override;

		virtual SharedRef<PhysicsBody> GetPhysicsBody(const std::string& boneName = std::string()) const override;

		virtual SharedRef<Mesh> GetMesh() const override
		{
			return m_SkeletalMesh;
		}

	private:
		SharedRef<SkeletalMesh> m_SkeletalMesh;

		std::map<std::string, SharedRef<PhysicsBody>> m_PhysicsBodyMap;
	};
} // namespace Neon
