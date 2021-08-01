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
		const physx::PxVec3& ToPhysXVector(const glm::vec3& vector);

		physx::PxBroadPhaseType::Enum NeonToPhysXBroadphaseType(BroadphaseType type);

		physx::PxFrictionType::Enum NeonToPhysXFrictionType(FrictionType type);



	} // namespace PhysXUtils

	class PhysXPhysics : public Physics
	{
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
