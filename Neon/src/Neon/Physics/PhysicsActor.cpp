#include "neopch.h"

#include "PhysicsActor.h"

namespace Neon
{
	PhysicsActor::PhysicsActor(Entity entity)
		: m_Entity(entity), m_RigidBody(entity.GetComponent<RigidBodyComponent>())
	{
	}

	PhysicsActor::~PhysicsActor()
	{
	}

	float PhysicsActor::GetMass() const
	{
		return m_RigidBody.Mass;
	}

	void PhysicsActor::SetMass(float mass)
	{
		m_RigidBody.Mass = mass;
	}

} // namespace Neon
