#pragma once

#include "Neon/Physics/Physics.h"
#include "Neon/Physics/PhysicsSettings.h"

#include <PhysX/PxPhysicsAPI.h>
#include <glm/gtc/type_ptr.hpp>

namespace Neon
{
	physx::PxForceMode::Enum GetPhysxForceModeFromNeonForceMode(ForceMode forceMode);

	namespace PhysXUtils
	{
		physx::PxTransform ToPhysXTransform(const Transform& transform);
		Transform FromPhysXTransform(const physx::PxTransform& physxTransform);

		const physx::PxVec3& ToPhysXVector(const glm::vec3& vector);
		glm::vec3 FromPhysXVector(const physx::PxVec3& vector);

		physx::PxQuat ToPhysXQuat(const glm::quat& quat);
		glm::quat FromPhysXQuat(const physx::PxQuat& quat);

		physx::PxBroadPhaseType::Enum NeonToPhysXBroadphaseType(BroadphaseType type);

		physx::PxFrictionType::Enum NeonToPhysXFrictionType(FrictionType type);

	} // namespace PhysXUtils

	class PhysXPhysics : public Physics
	{
	public:
		static physx::PxFoundation& GetFoundation();

	public:
		PhysXPhysics() = default;
		virtual ~PhysXPhysics() = default;

		physx::PxDefaultCpuDispatcher* GetCPUDispatcher() const;

	protected:
		virtual void* InternalGetPhysicsSDK() const override;

		virtual void InternalInitialize() override;
		virtual void InternalShutdown() override;

		virtual void InternalCreateScene() override;
		virtual void InternalDestroyScene() override;
	};
} // namespace Neon
