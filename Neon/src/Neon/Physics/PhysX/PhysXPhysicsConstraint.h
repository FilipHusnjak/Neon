#pragma once

#include "Neon/Physics/PhysicsConstraint.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXPhysicsConstraint : public PhysicsConstraint
	{
	public:
		PhysXPhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1);
		virtual ~PhysXPhysicsConstraint() = default;

		virtual void* GetHandle() const override
		{
			NEO_CORE_ASSERT(m_Joint);
			return m_Joint;
		}

		virtual void Destroy() override;

		virtual void SetMotion(MotionAxis motionAxis, Motion motion) override;
		virtual void SetDrive(DriveAxis driveAxis, float driveStiffness, float driveDamping, float driveForceLimit) override;
		virtual void SetDrivePosition(const Transform& transform) override;
		virtual void SetDriveVelocity(const glm::vec3& linear, const glm::vec3& angular) override;
		virtual void SetLinearLimit(float limit) override;

	private:
		physx::PxD6Joint* m_Joint = nullptr;
	};
} // namespace Neon
