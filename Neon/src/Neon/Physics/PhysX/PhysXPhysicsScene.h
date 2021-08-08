#pragma once

#include "Neon/Physics/PhysicsScene.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXPhysicsScene : public PhysicsScene
	{
	public:
		PhysXPhysicsScene(const PhysicsSettings& settings);
		virtual ~PhysXPhysicsScene() override;

		virtual void Tick(float deltaSeconds) override;

		virtual void Destroy() override;

		virtual SharedRef<PhysicsConstraint> AddPhysicsConstraint(const SharedRef<PhysicsBody>& body0,
																  const SharedRef<PhysicsBody>& body1) override;
		virtual void RemovePhysicsConstraint(SharedRef<PhysicsConstraint>& physicsConstraint);

		virtual SharedRef<PhysicsBody> AddPhysicsBody(PhysicsBodyType physicsBodyType, const Transform& transform, const SharedRef<PhysicsMaterial>& material) override;
		virtual void RemovePhysicsBody(SharedRef<PhysicsBody>& physicsBody) override;

	private:
		physx::PxScene* m_PhysXScene;
	};
} // namespace Neon
