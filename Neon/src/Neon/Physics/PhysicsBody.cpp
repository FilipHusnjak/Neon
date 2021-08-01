#include "neopch.h"

#include "Neon/Physics/PhysicsActor.h"
#include "Neon/Physics/PhysicsBody.h"

namespace Neon
{
	PhysicsBody::PhysicsBody(const PhysicsActor& physicsActor)
		: m_PhysicsActor(physicsActor)
	{
	}

	void PhysicsBody::SetMaterial(const SharedRef<PhysicsMaterial>& material)
	{
		m_Material = material;
	}

} // namespace Neon
