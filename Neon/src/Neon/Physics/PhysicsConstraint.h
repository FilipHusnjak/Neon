#pragma once

#include "Neon/Physics/PhysicsBody.h"

namespace Neon
{
	enum class MotionAxis
	{
		TranslationX,
		TranslationY,
		TranslationZ,

		RotationX,
		RotationY,
		RotationZ
	};

	enum class DriveAxis
	{
		TranslationX,
		TranslationY,
		TranslationZ,

		Swing,
		Twist,
		Slerp
	};

	enum class Motion
	{
		Locked,
		Limited,
		Free
	};

	class PhysicsConstraint : public RefCounted
	{
	public:
		static SharedRef<PhysicsConstraint> Create(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1);

	public:
		PhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1);
		virtual ~PhysicsConstraint() = default;

		virtual void* GetHandle() const = 0;

		virtual void Destroy() = 0;

		virtual void SetMotion(MotionAxis motionAxis, Motion motion) = 0;
		virtual void SetDrive(DriveAxis driveAxis, float driveStiffness, float driveDamping, float driveForceLimit) = 0;
		virtual void SetDrivePosition(const Transform& transform) = 0;
		virtual void SetDriveVelocity(const glm::vec3& linear, const glm::vec3& angular) = 0;
		virtual void SetLinearLimit(float limit) = 0;

	protected:
		SharedRef<PhysicsBody> m_Body0;
		SharedRef<PhysicsBody> m_Body1;
	};
} // namespace Neon
