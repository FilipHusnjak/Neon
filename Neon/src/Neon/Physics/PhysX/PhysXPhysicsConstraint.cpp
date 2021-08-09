#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsConstraint.h"

namespace Neon
{
	static physx::PxD6Axis::Enum FromNeonMotionAxis(MotionAxis motionAxis)
	{
		switch (motionAxis)
		{
			case MotionAxis::TranslationX:
				return physx::PxD6Axis::Enum::eX;
			case MotionAxis::TranslationY:
				return physx::PxD6Axis::Enum::eY;
			case MotionAxis::TranslationZ:
				return physx::PxD6Axis::Enum::eZ;

			case MotionAxis::RotationX:
				return physx::PxD6Axis::Enum::eTWIST;
			case MotionAxis::RotationY:
				return physx::PxD6Axis::Enum::eSWING1;
			case MotionAxis::RotationZ:
				return physx::PxD6Axis::Enum::eSWING2;

			default:
				NEO_CORE_ASSERT(false);
				return physx::PxD6Axis::Enum::eCOUNT;
		}
	}

	static physx::PxD6Motion::Enum FromNeonMotion(Motion motion)
	{
		switch (motion)
		{
			case Motion::Locked:
				return physx::PxD6Motion::Enum::eLOCKED;
			case Motion::Limited:
				return physx::PxD6Motion::Enum::eLIMITED;
			case Motion::Free:
				return physx::PxD6Motion::Enum::eFREE;

			default:
				NEO_CORE_ASSERT(false);
				return physx::PxD6Motion::Enum::eFREE;
		}
	}

	static physx::PxD6Drive::Enum FromNeonDriveAxis(DriveAxis driveAxis)
	{
		switch (driveAxis)
		{
			case DriveAxis::TranslationX:
				return physx::PxD6Drive::Enum::eX;
			case DriveAxis::TranslationY:
				return physx::PxD6Drive::Enum::eY;
			case DriveAxis::TranslationZ:
				return physx::PxD6Drive::Enum::eZ;

			case DriveAxis::Twist:
				return physx::PxD6Drive::Enum::eTWIST;
			case DriveAxis::Swing:
				return physx::PxD6Drive::Enum::eSWING;
			case DriveAxis::Slerp:
				return physx::PxD6Drive::Enum::eSLERP;

			default:
				NEO_CORE_ASSERT(false);
				return physx::PxD6Drive::Enum::eCOUNT;
		}
	}

	PhysXPhysicsConstraint::PhysXPhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1)
		: PhysicsConstraint(body0, body1)
	{
		m_Joint = physx::PxD6JointCreate(
			*static_cast<physx::PxPhysics*>(Physics::GetPhysicsSDK()), static_cast<physx::PxRigidActor*>(body1->GetHandle()),
			PhysXUtils::ToPhysXTransform(body0->GetBodyTransform() * body1->GetBodyTransform().Inverse()),
			static_cast<physx::PxRigidActor*>(body0->GetHandle()), PhysXUtils::ToPhysXTransform(Transform()));

		NEO_CORE_ASSERT(m_Joint);

		m_Joint->setProjectionLinearTolerance(0.001f);
		m_Joint->setProjectionAngularTolerance(1.f);
	}

	void PhysXPhysicsConstraint::Destroy()
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->release();
		m_Joint = nullptr;
	}

	void PhysXPhysicsConstraint::SetMotion(MotionAxis motionAxis, Motion motion)
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->setMotion(FromNeonMotionAxis(motionAxis), FromNeonMotion(motion));
	}

	void PhysXPhysicsConstraint::SetDrive(DriveAxis driveAxis, float driveStiffness, float driveDamping, float driveForceLimit)
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->setDrive(FromNeonDriveAxis(driveAxis), physx::PxD6JointDrive(driveStiffness, driveDamping, driveForceLimit));
	}

	void PhysXPhysicsConstraint::SetDrivePosition(const Transform& transform)
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->setDrivePosition(PhysXUtils::ToPhysXTransform(transform));
	}

	void PhysXPhysicsConstraint::SetDriveVelocity(const glm::vec3& linear, const glm::vec3& angular)
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->setDriveVelocity(PhysXUtils::ToPhysXVector(linear), PhysXUtils::ToPhysXVector(angular));
	}

	void PhysXPhysicsConstraint::SetLinearLimit(float limit)
	{
		NEO_CORE_ASSERT(m_Joint);

		m_Joint->setLinearLimit(physx::PxJointLinearLimit(physx::PxTolerancesScale(), limit));
	}

} // namespace Neon
