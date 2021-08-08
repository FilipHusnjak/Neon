#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsBody.h"
#include "Neon/Physics/PhysX/PhysXPhysicsMaterial.h"
#include "Neon/Physics/PhysX/PhysXPhysicsPrimitives.h"

#include <PhysX/extensions/PxRigidBodyExt.h>

namespace Neon
{
	PhysXPhysicsBody::PhysXPhysicsBody(PhysicsBodyType bodyType, const Transform& transform,
									   const SharedRef<PhysicsMaterial>& material)
		: PhysicsBody(bodyType, transform, material)
	{
		auto* sdk = static_cast<physx::PxPhysics*>(Physics::GetPhysicsSDK());
		NEO_CORE_ASSERT(sdk);

		if (bodyType == PhysicsBodyType::Static)
		{
			m_RigidActor = sdk->createRigidStatic(PhysXUtils::ToPhysXTransform(transform));
		}
		else
		{
			m_RigidActor = sdk->createRigidDynamic(PhysXUtils::ToPhysXTransform(transform));
			m_RigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
		}
	}

	void PhysXPhysicsBody::Destroy()
	{
		NEO_CORE_ASSERT(m_RigidActor);

		m_RigidActor->release();
		m_RigidActor = nullptr;
	}

	void PhysXPhysicsBody::AddSpherePrimitive(float radius /*= 1.f*/, const Transform& transform)
	{
		m_Spheres.emplace_back(CreateUnique<PhysXSpherePhysicsPrimitive>(*this, radius, transform));

		if (m_BodyType == PhysicsBodyType::Dynamic)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*static_cast<physx::PxRigidDynamic*>(m_RigidActor), m_Material->Density);
		}
	}

	void PhysXPhysicsBody::AddBoxPrimitive(glm::vec3 size, const Transform& transform)
	{
		m_Boxes.emplace_back(CreateUnique<PhysXBoxPhysicsPrimitive>(*this, size, transform));
		
		if (m_BodyType == PhysicsBodyType::Dynamic)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*static_cast<physx::PxRigidDynamic*>(m_RigidActor), m_Material->Density);
		}
	}

	void PhysXPhysicsBody::AddCapsulePrimitive(float radius, float height, const Transform& transform)
	{
		m_Capsules.emplace_back(CreateUnique<PhysXCapsulePhysicsPrimitive>(*this, radius, height, transform));
		
		if (m_BodyType == PhysicsBodyType::Dynamic)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*static_cast<physx::PxRigidDynamic*>(m_RigidActor), m_Material->Density);
		}
	}

	void PhysXPhysicsBody::AddForce(const glm::vec3& force)
	{
		NEO_CORE_ASSERT(m_BodyType == PhysicsBodyType::Dynamic);

		m_RigidActor->is<physx::PxRigidDynamic>()->addForce(PhysXUtils::ToPhysXVector(force));
	}

	float PhysXPhysicsBody::GetMass() const
	{
		NEO_CORE_ASSERT(m_RigidActor->is<physx::PxRigidDynamic>());
		return m_RigidActor->is<physx::PxRigidDynamic>()->getMass();
	}

	void PhysXPhysicsBody::SetMass(float mass)
	{
		NEO_CORE_ASSERT(m_RigidActor->is<physx::PxRigidDynamic>());
		m_RigidActor->is<physx::PxRigidDynamic>()->setMass(mass);
	}

	Transform PhysXPhysicsBody::GetBodyTransform() const
	{
		Transform transform = PhysXUtils::FromPhysXTransform(m_RigidActor->getGlobalPose());
		return transform;
	}

	glm::vec3 PhysXPhysicsBody::GetBodyTranslation() const
	{
		Transform transform = PhysXUtils::FromPhysXTransform(m_RigidActor->getGlobalPose());
		return transform.Translation;
	}

	glm::quat PhysXPhysicsBody::GetBodyRotation() const
	{
		Transform transform = PhysXUtils::FromPhysXTransform(m_RigidActor->getGlobalPose());
		return transform.Rotation;
	}

} // namespace Neon
