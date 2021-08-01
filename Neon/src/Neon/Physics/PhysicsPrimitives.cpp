#include "neopch.h"

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Physics/PhysicsPrimitives.h"

namespace Neon
{
	PhysicsPrimitive::PhysicsPrimitive(const PhysicsBody& physicsBody)
		: m_PhysicsBody(physicsBody)
		, m_InternalShape(nullptr)
	{
	}

	SpherePhysicsPrimitive::SpherePhysicsPrimitive(const PhysicsBody& physicsBody, float radius)
		: PhysicsPrimitive(physicsBody)
		, m_Radius(radius)
		, m_Offset(glm::vec3())
	{
	}
} // namespace Neon
