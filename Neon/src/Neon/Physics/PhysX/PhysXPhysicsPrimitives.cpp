#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsPrimitives.h"

namespace Neon
{
	PhysXSpherePhysicsPrimitive::PhysXSpherePhysicsPrimitive(const PhysicsBody& physicsBody, float radius,
															 const Transform& transform)
		: SpherePhysicsPrimitive(physicsBody, radius, transform)
	{
		NEO_CORE_ASSERT(physicsBody.GetMaterial());

		physx::PxSphereGeometry geometry = physx::PxSphereGeometry(radius);
		physx::PxShape* shape =
			physx::PxRigidActorExt::createExclusiveShape(*static_cast<physx::PxRigidActor*>(physicsBody.GetHandle()), geometry,
														 *static_cast<physx::PxMaterial*>(physicsBody.GetMaterial()->GetHandle()));
		m_InternalShape = shape;
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
		shape->setLocalPose(PhysXUtils::ToPhysXTransform(transform));
	}

	PhysXBoxPhysicsPrimitive::PhysXBoxPhysicsPrimitive(const PhysicsBody& physicsBody, glm::vec3 size,
													   const Transform& transform)
		: BoxPhysicsPrimitive(physicsBody, size, transform)
	{
		NEO_CORE_ASSERT(physicsBody.GetMaterial());

		physx::PxBoxGeometry geometry = physx::PxBoxGeometry(size.x, size.y, size.z);
		physx::PxShape* shape =
			physx::PxRigidActorExt::createExclusiveShape(*static_cast<physx::PxRigidActor*>(physicsBody.GetHandle()), geometry,
														 *static_cast<physx::PxMaterial*>(physicsBody.GetMaterial()->GetHandle()));
		m_InternalShape = shape;
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
		shape->setLocalPose(PhysXUtils::ToPhysXTransform(transform));
	}

	PhysXCapsulePhysicsPrimitive::PhysXCapsulePhysicsPrimitive(const PhysicsBody& physicsBody, float radius, float height,
															   const Transform& transform)
		: CapsulePhysicsPrimitive(physicsBody, radius, height, transform)
	{
		NEO_CORE_ASSERT(physicsBody.GetMaterial());

		physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(radius, height / 2.f);
		physx::PxShape* shape =
			physx::PxRigidActorExt::createExclusiveShape(*static_cast<physx::PxRigidActor*>(physicsBody.GetHandle()), geometry,
														 *static_cast<physx::PxMaterial*>(physicsBody.GetMaterial()->GetHandle()));
		m_InternalShape = shape;
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
		shape->setLocalPose(PhysXUtils::ToPhysXTransform(transform));
	}

} // namespace Neon
