#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsPrimitives.h"

namespace Neon
{
	PhysXSpherePhysicsPrimitive::PhysXSpherePhysicsPrimitive(const PhysicsBody& physicsBody, float radius)
		: SpherePhysicsPrimitive(physicsBody, radius)
	{
		NEO_CORE_ASSERT(physicsBody.GetMaterial());

		physx::PxSphereGeometry geometry = physx::PxSphereGeometry(radius);
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(
			*static_cast<physx::PxRigidActor*>(physicsBody.GetHandle()), geometry,
			*static_cast<physx::PxMaterial*>(physicsBody.GetMaterial()->GetHandle()));
		m_InternalShape = shape;
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);

		// TODO
		//shape->setLocalPose(::ToPhysXTransform(offset + m_Component.Offset, glm::vec3(0.0f)));
	}
} // namespace Neon
