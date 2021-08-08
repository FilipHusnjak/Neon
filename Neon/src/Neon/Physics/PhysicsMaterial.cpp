#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsMaterial.h"
#include "Neon/Physics/Physics.h"
#include "Neon/Physics/PhysicsMaterial.h"

namespace Neon
{
	SharedRef<PhysicsMaterial> PhysicsMaterial::CreateMaterial(float staticFriction, float dynamicFriction, float restitution,
															   float density)
	{
		if (Physics::GetCurrentEngine() == PhysicsEngine::PhysX)
		{
			return SharedRef<PhysXPhysicsMaterial>::Create(staticFriction, dynamicFriction, restitution, density);
		}
		else
		{
			NEO_CORE_ASSERT(false, "Uknown physics engine selected!");
			return nullptr;
		}
	}

	PhysicsMaterial::PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution, float density)
		: StaticFriction(staticFriction)
		, DynamicFriction(dynamicFriction)
		, Restitution(restitution)
		, Density(density)
	{
	}
} // namespace Neon
