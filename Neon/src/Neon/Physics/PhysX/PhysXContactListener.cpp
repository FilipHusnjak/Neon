#include "neopch.h"

#include "PhysXContactListener.h"

namespace Neon
{
	void PhysXContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		// TODO

		PX_UNUSED(constraints);
		PX_UNUSED(count);
	}

	void PhysXContactListener::onWake(physx::PxActor** actors, physx::PxU32 count)
	{
		// TODO

		PX_UNUSED(actors);
		PX_UNUSED(count);
	}

	void PhysXContactListener::onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
		// TODO

		PX_UNUSED(actors);
		PX_UNUSED(count);
	}

	void PhysXContactListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs,
										 physx::PxU32 nbPairs)
	{
		// TODO

		PX_UNUSED(pairHeader);
		PX_UNUSED(pairs);
		PX_UNUSED(nbPairs);
	}

	void PhysXContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		// TODO

		PX_UNUSED(pairs);
		PX_UNUSED(count);
	}

	void PhysXContactListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer,
										 const physx::PxU32 count)
	{
		// TODO

		PX_UNUSED(bodyBuffer);
		PX_UNUSED(poseBuffer);
		PX_UNUSED(count);
	}

} // namespace Neon
