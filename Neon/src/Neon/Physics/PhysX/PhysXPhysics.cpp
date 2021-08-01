#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXErrorCallback.h"
#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsScene.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	static PhysXErrorCallback s_ErrorCallback;
	static physx::PxDefaultAllocator s_Allocator;
	static physx::PxFoundation* s_Foundation = nullptr;
	static physx::PxPvd* s_PVD = nullptr;
	static physx::PxPhysics* s_Physics = nullptr;
	static physx::PxCooking* s_CookingFactory = nullptr;
	static physx::PxOverlapHit s_OverlapBuffer[OVERLAP_MAX_COLLIDERS];
	static physx::PxDefaultCpuDispatcher* s_CPUDispatcher = nullptr;

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

		s_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Foundation, scale, s_TrackMemoryAllocations, s_PVD);
		NEO_CORE_ASSERT(s_Physics, "PxCreatePhysics failed.");

		bool extentionsLoaded = PxInitExtensions(*s_Physics, s_PVD);
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

		if (s_Physics)
			s_Physics->release();
		s_Physics = nullptr;

		if (s_Foundation)
			s_Foundation->release();
		s_Foundation = nullptr;
	}

	void PhysXPhysics::InternalCreateScene()
	{
		s_Scene = SharedRef<PhysXPhysicsScene>::Create(s_Settings);
	}

	void PhysXPhysics::InternalDestroyScene()
	{
	}

	const physx::PxVec3& PhysXUtils::ToPhysXVector(const glm::vec3& vector)
	{
		return *(physx::PxVec3*)&vector;
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
