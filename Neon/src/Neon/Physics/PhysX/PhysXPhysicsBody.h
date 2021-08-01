#pragma once

#include "Neon/Physics/PhysicsBody.h"

namespace Neon
{
	class PhysXPhysicsBody : public PhysicsBody
	{
	public:
		PhysXPhysicsBody(const PhysicsActor& physicsActor);
		virtual void AddSpherePrimitive(float radius = 1.f) override;
	};
} // namespace Neon
