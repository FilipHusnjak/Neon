#pragma once

#include "Neon/Physics/PhysicsBody.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXPhysicsBody : public PhysicsBody
	{
	public:
		PhysXPhysicsBody();

		virtual void Destroy() override;

		virtual void AddSpherePrimitive(float radius = 1.f) override;

		virtual void* GetHandle() const override
		{
			return m_RigidActor;
		}

	private:
		physx::PxRigidActor* m_RigidActor = nullptr;
	};
} // namespace Neon
