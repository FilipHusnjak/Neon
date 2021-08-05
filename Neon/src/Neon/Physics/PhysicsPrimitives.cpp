#include "neopch.h"

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Physics/PhysicsPrimitives.h"

namespace Neon
{
	PhysicsPrimitive::PhysicsPrimitive(const PhysicsBody& physicsBody)
		: m_InternalShape(nullptr)
	{
	}

	SpherePhysicsPrimitive::SpherePhysicsPrimitive(const PhysicsBody& physicsBody, float radius)
		: PhysicsPrimitive(physicsBody)
		, m_Radius(radius)
	{
	}

	BoxPhysicsPrimitive::BoxPhysicsPrimitive(const PhysicsBody& physicsBody, glm::vec3 size)
		: PhysicsPrimitive(physicsBody)
		, m_Size(size)
	{
	}

	CapsulePhysicsPrimitive::CapsulePhysicsPrimitive(const PhysicsBody& physicsBody, float radius, float height)
		: PhysicsPrimitive(physicsBody)
		, m_Radius(radius)
		, m_Height(height)
	{
	}

} // namespace Neon
