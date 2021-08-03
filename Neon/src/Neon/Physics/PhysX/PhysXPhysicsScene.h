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

		virtual void Destroy() override;

		virtual SharedRef<PhysicsBody> InternalAddPhysicsBody() override;
		virtual void InternalRemovePhysicsBody(SharedRef<PhysicsBody> actor) override;

	private:
		physx::PxScene* m_PhysXScene;
	};
} // namespace Neon