#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsBody.h"
#include "Neon/Physics/PhysX/PhysXPhysicsMaterial.h"
#include "Neon/Physics/PhysX/PhysXPhysicsPrimitives.h"
#include "Neon/Physics/Physics.h"

namespace Neon
{
	PhysXPhysicsBody::PhysXPhysicsBody()
	{
		auto* sdk = static_cast<physx::PxPhysics*>(Physics::GetPhysicsSDK());
		NEO_CORE_ASSERT(sdk);

		m_RigidActor = sdk->createRigidStatic(physx::PxTransform(physx::PxIDENTITY::PxIdentity));

		SetMaterial(SharedRef<PhysXPhysicsMaterial>::Create(1.f, 0.7f, 0.5f));
	}

	void PhysXPhysicsBody::Destroy()
	{
		NEO_CORE_ASSERT(m_RigidActor);

		m_RigidActor->release();
		m_RigidActor = nullptr;
	}

	void PhysXPhysicsBody::AddSpherePrimitive(float radius /*= 1.f*/)
	{
		m_Spheres.emplace_back(CreateUnique<PhysXSpherePhysicsPrimitive>(*this, 10.f));
	}
} // namespace Neon
