#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsConstraint.h"

namespace Neon
{
	PhysXPhysicsConstraint::PhysXPhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1)
		: PhysicsConstraint(body0, body1)
	{
		m_Joint = physx::PxD6JointCreate(
			*static_cast<physx::PxPhysics*>(Physics::GetPhysicsSDK()), static_cast<physx::PxRigidActor*>(body0->GetHandle()),
			PhysXUtils::ToPhysXTransform(body1->GetBodyTransform() * body0->GetBodyTransform().Inverse()),
			static_cast<physx::PxRigidActor*>(body1->GetHandle()), PhysXUtils::ToPhysXTransform(Transform()));

		NEO_CORE_ASSERT(m_Joint);

		m_Joint->setMotion(physx::PxD6Axis::eSWING1, physx::PxD6Motion::eFREE);
		m_Joint->setProjectionLinearTolerance(0.1f);
		m_Joint->setProjectionAngularTolerance(1.f);

		m_Joint->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eLIMITED);
		m_Joint->setLinearLimit(physx::PxJointLinearLimit(physx::PxTolerancesScale(), 0.05f));
		m_Joint->setDrivePosition(physx::PxTransform(physx::PxIDENTITY()));
		m_Joint->setDriveVelocity(physx::PxVec3(physx::PxZero), physx::PxVec3(physx::PxZero));
	}

	void PhysXPhysicsConstraint::Destroy()
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->release();
		m_Joint = nullptr;
	}

} // namespace Neon
