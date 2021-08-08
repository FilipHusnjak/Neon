#include "neopch.h"

#include "Neon/Physics/Physics.h"
#include "PhysXPhysicsMaterial.h"

namespace Neon
{

	PhysXPhysicsMaterial::PhysXPhysicsMaterial(float staticFriction, float dynamicFriction, float restitution, float density)
		: PhysicsMaterial(staticFriction, dynamicFriction, restitution, density)
	{
		physx::PxPhysics* physSDK = static_cast<physx::PxPhysics*>(Physics::GetPhysicsSDK());
		m_Handle = physSDK->createMaterial(staticFriction, dynamicFriction, restitution);
	}

} // namespace Neon
