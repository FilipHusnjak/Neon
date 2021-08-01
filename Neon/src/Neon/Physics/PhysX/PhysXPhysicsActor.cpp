#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsActor.h"
#include "Neon/Physics/Physics.h"

namespace Neon
{
	PhysXPhysicsActor::PhysXPhysicsActor(Entity entity)
		: PhysicsActor(entity)
	{
		auto* sdk = static_cast<physx::PxPhysics*>(Physics::GetPhysicsSDK());
		NEO_CORE_ASSERT(sdk);
		
		m_RigidActor = sdk->createRigidStatic(physx::PxTransform(physx::PxIDENTITY::PxIdentity));
	}

} // namespace Neon
