#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsPrimitives.h"

namespace Neon
{
	PhysXSpherePhysicsPrimitive::PhysXSpherePhysicsPrimitive(const PhysicsBody& physicsBody, float radius)
		: SpherePhysicsPrimitive(physicsBody, radius)
	{
		NEO_CORE_ASSERT(physicsBody.GetMaterial());

		physx::PxSphereGeometry geometry = physx::PxSphereGeometry(radius);
		physx::PxShape* shape =
			physx::PxRigidActorExt::createExclusiveShape(*static_cast<physx::PxRigidActor*>(physicsBody.GetHandle()), geometry,
														 *static_cast<physx::PxMaterial*>(physicsBody.GetMaterial()->GetHandle()));
		m_InternalShape = shape;
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);

		// TODO
		//shape->setLocalPose(::ToPhysXTransform(offset + m_Component.Offset, glm::vec3(0.0f)));
	}

	PhysXBoxPhysicsPrimitive::PhysXBoxPhysicsPrimitive(const PhysicsBody& physicsBody, glm::vec3 size)
		: BoxPhysicsPrimitive(physicsBody, size)
	{
		NEO_CORE_ASSERT(physicsBody.GetMaterial());

		physx::PxBoxGeometry geometry = physx::PxBoxGeometry(size.x, size.y, size.z);
		physx::PxShape* shape =
			physx::PxRigidActorExt::createExclusiveShape(*static_cast<physx::PxRigidActor*>(physicsBody.GetHandle()), geometry,
														 *static_cast<physx::PxMaterial*>(physicsBody.GetMaterial()->GetHandle()));
		m_InternalShape = shape;
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);

		// TODO
		//shape->setLocalPose(::ToPhysXTransform(offset + m_Component.Offset, glm::vec3(0.0f)));
	}

	PhysXCapsulePhysicsPrimitive::PhysXCapsulePhysicsPrimitive(const PhysicsBody& physicsBody, float radius, float height)
		: CapsulePhysicsPrimitive(physicsBody, radius, height)
	{
		NEO_CORE_ASSERT(physicsBody.GetMaterial());

		physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(radius, height / 2.f);
		physx::PxShape* shape =
			physx::PxRigidActorExt::createExclusiveShape(*static_cast<physx::PxRigidActor*>(physicsBody.GetHandle()), geometry,
														 *static_cast<physx::PxMaterial*>(physicsBody.GetMaterial()->GetHandle()));
		m_InternalShape = shape;
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);

		// TODO
		//shape->setLocalPose(::ToPhysXTransform(offset + m_Component.Offset, glm::vec3(0.0f)));
	}

} // namespace Neon
