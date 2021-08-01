#pragma once

#include "Neon/Physics/PhysicsActor.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXPhysicsActor : public PhysicsActor
	{
	public:
		PhysXPhysicsActor(Entity entity);
		virtual void* GetHandle() const override
		{
			return m_RigidActor;
		}

	private:
		physx::PxRigidActor* m_RigidActor = nullptr;
	};
} // namespace Neon
