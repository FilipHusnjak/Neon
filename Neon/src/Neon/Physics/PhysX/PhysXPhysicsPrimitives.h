#pragma once

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Physics/PhysicsPrimitives.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXSpherePhysicsPrimitive : public SpherePhysicsPrimitive
	{
	public:
		PhysXSpherePhysicsPrimitive(const PhysicsBody& physicsBody, float radius = 1.f);
	};

	class PhysXBoxPhysicsPrimitive : public BoxPhysicsPrimitive
	{
	public:
		PhysXBoxPhysicsPrimitive(const PhysicsBody& physicsBody, glm::vec3 size);
	};
} // namespace Neon
