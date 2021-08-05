#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXErrorCallback.h"
#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsDebugger.h"
#include "Neon/Physics/PhysX/PhysXPhysicsScene.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	static PhysXErrorCallback s_ErrorCallback;
	static physx::PxDefaultAllocator s_Allocator;
	static physx::PxFoundation* s_Foundation = nullptr;
	static physx::PxPhysics* s_Physics = nullptr;
	static physx::PxCooking* s_CookingFactory = nullptr;
	static physx::PxOverlapHit s_OverlapBuffer[OVERLAP_MAX_COLLIDERS];
	static physx::PxDefaultCpuDispatcher* s_CPUDispatcher = nullptr;

	physx::PxFoundation& PhysXPhysics::GetFoundation()
	{
		NEO_CORE_ASSERT(s_Foundation);

		return *s_Foundation;
	}

	physx::PxDefaultCpuDispatcher* PhysXPhysics::GetCPUDispatcher() const
	{
		NEO_CORE_ASSERT(s_CPUDispatcher);

		return s_CPUDispatcher;
	}

	void* PhysXPhysics::InternalGetPhysicsSDK() const
	{
		return s_Physics;
	}

	void PhysXPhysics::InternalInitialize()
	{
		NEO_CORE_ASSERT(!s_Foundation, "Trying to initialize the PhysX SDK multiple times!");

		s_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Allocator, s_ErrorCallback);
		NEO_CORE_ASSERT(s_Foundation, "PxCreateFoundation failed.");

		physx::PxTolerancesScale scale = physx::PxTolerancesScale();
		scale.length = 1.0f;
		scale.speed = 9.81f;

#ifdef NEO_DEBUG
		static bool s_TrackMemoryAllocations = true;
#else
		static bool s_TrackMemoryAllocations = false;
#endif

		PhysXPhysicsDebugger::Initialize();

		s_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Foundation, scale, s_TrackMemoryAllocations, PhysXPhysicsDebugger::GetDebugger());
		NEO_CORE_ASSERT(s_Physics, "PxCreatePhysics failed.");

		bool extentionsLoaded = PxInitExtensions(*s_Physics, PhysXPhysicsDebugger::GetDebugger());
		NEO_CORE_ASSERT(extentionsLoaded, "Failed to initialize PhysX Extensions.");

		s_CPUDispatcher = physx::PxDefaultCpuDispatcherCreate(1);

		s_CookingFactory = PxCreateCooking(PX_PHYSICS_VERSION, *s_Foundation, s_Physics->getTolerancesScale());
		NEO_CORE_ASSERT(s_CookingFactory, "PxCreatePhysics Failed!");
	}

	void PhysXPhysics::InternalShutdown()
	{
		if (s_CPUDispatcher)
			s_CPUDispatcher->release();
		s_CPUDispatcher = nullptr;

		if (s_CookingFactory)
			s_CookingFactory->release();
		s_CookingFactory = nullptr;

		PxCloseExtensions();

		PhysXPhysicsDebugger::StopDebugging();

		if (s_Physics)
			s_Physics->release();
		s_Physics = nullptr;

		PhysXPhysicsDebugger::StopDebugging();

		if (s_Foundation)
			s_Foundation->release();
		s_Foundation = nullptr;
	}

	void PhysXPhysics::InternalCreateScene()
	{
		s_Scene = SharedRef<PhysXPhysicsScene>::Create(s_Settings);

		PhysXPhysicsDebugger::StartDebugging();
	}

	void PhysXPhysics::InternalDestroyScene()
	{
	}

	physx::PxTransform PhysXUtils::ToPhysXTransform(const Transform& transform)
	{
		physx::PxQuat r = ToPhysXQuat(glm::quat(transform.Rotation));
		physx::PxVec3 p = ToPhysXVector(transform.Translation);
		return physx::PxTransform(p, r);
	}

	Transform PhysXUtils::FromPhysXTransform(const physx::PxTransform& physxTransform)
	{
		return Transform(PhysXUtils::FromPhysXVector(physxTransform.p),
						 glm::eulerAngles(PhysXUtils::FromPhysXQuat(physxTransform.q)), glm::vec3(1.f));
	}

	const physx::PxVec3& PhysXUtils::ToPhysXVector(const glm::vec3& vector)
	{
		return *(physx::PxVec3*)&vector;
	}

	glm::vec3 PhysXUtils::FromPhysXVector(const physx::PxVec3& vector)
	{
		return *(glm::vec3*)&vector;
	}

	physx::PxQuat PhysXUtils::ToPhysXQuat(const glm::quat& quat)
	{
		return physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
	}

	glm::quat PhysXUtils::FromPhysXQuat(const physx::PxQuat& quat)
	{
		return *(glm::quat*)&quat;
	}

	physx::PxBroadPhaseType::Enum PhysXUtils::NeonToPhysXBroadphaseType(BroadphaseType type)
	{
		switch (type)
		{
			case BroadphaseType::SweepAndPrune:
				return physx::PxBroadPhaseType::eSAP;
			case BroadphaseType::MultiBoxPrune:
				return physx::PxBroadPhaseType::eMBP;
			case BroadphaseType::AutomaticBoxPrune:
				return physx::PxBroadPhaseType::eABP;
		}

		return physx::PxBroadPhaseType::eABP;
	}

	physx::PxFrictionType::Enum PhysXUtils::NeonToPhysXFrictionType(FrictionType type)
	{
		switch (type)
		{
			case FrictionType::Patch:
				return physx::PxFrictionType::ePATCH;
			case FrictionType::OneDirectional:
				return physx::PxFrictionType::eONE_DIRECTIONAL;
			case FrictionType::TwoDirectional:
				return physx::PxFrictionType::eTWO_DIRECTIONAL;
		}

		return physx::PxFrictionType::ePATCH;
	}

	physx::PxForceMode::Enum GetPhysxForceModeFromNeonForceMode(ForceMode forceMode)
	{
		return (physx::PxForceMode::Enum)forceMode;
	}

} // namespace Neon
