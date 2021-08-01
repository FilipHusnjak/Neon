#include "neopch.h"

#include "PhysicsMaterial.h"

namespace Neon
{
	PhysicsMaterial::PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution)
		: StaticFriction(staticFriction)
		, DynamicFriction(dynamicFriction)
		, Restitution(restitution)
	{
	}
} // namespace Neon
