#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsConstraint.h"
#include "Neon/Physics/PhysX/PhysXPhysics.h"

namespace Neon
{
	PhysXPhysicsConstraint::PhysXPhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1)
		: PhysicsConstraint(body0, body1)
	{
		m_Joint = physx::PxFixedJointCreate(
			*static_cast<physx::PxPhysics*>(Physics::GetPhysicsSDK()), static_cast<physx::PxRigidActor*>(body0->GetHandle()),
			physx::PxTransform(PhysXUtils::ToPhysXTransform({glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(1.f)})), static_cast<physx::PxRigidActor*>(body1->GetHandle()),
			physx::PxTransform(physx::PxIDENTITY()));
	}

	void PhysXPhysicsConstraint::Destroy()
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->release();
		m_Joint = nullptr;
	}

} // namespace Neon
