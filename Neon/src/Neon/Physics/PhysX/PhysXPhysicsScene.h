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

		virtual SharedRef<PhysicsActor> InternalCreateActor(Entity entity) override;
		void InternalRemoveActor(SharedRef<PhysicsActor> actor) override;

	private:
		physx::PxScene* m_PhysXScene;
	};
} // namespace Neon
