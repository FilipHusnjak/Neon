#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsBody.h"
#include "Neon/Physics/PhysX/PhysXPhysicsMaterial.h"
#include "Neon/Physics/PhysX/PhysXPhysicsPrimitives.h"

namespace Neon
{
	PhysXPhysicsBody::PhysXPhysicsBody(const PhysicsActor& physicsActor)
		: PhysicsBody(physicsActor)
	{
		SetMaterial(SharedRef<PhysXPhysicsMaterial>::Create(1.f, 0.7f, 0.5f));
	}

	void PhysXPhysicsBody::AddSpherePrimitive(float radius /*= 1.f*/)
	{
		m_Spheres.emplace_back(CreateUnique<PhysXSpherePhysicsPrimitive>(*this, 10.f));
	}
} // namespace Neon
