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
			PhysXUtils::ToPhysXTransform(body0->GetBodyTransform() * body1->GetBodyTransform().Inverse()),
			static_cast<physx::PxRigidActor*>(body1->GetHandle()), PhysXUtils::ToPhysXTransform(Transform()));

		NEO_CORE_ASSERT(m_Joint);
	}

	void PhysXPhysicsConstraint::Destroy()
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->release();
		m_Joint = nullptr;
	}

} // namespace Neon
