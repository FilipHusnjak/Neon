#pragma once

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXPhysicsDebugger
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void StartDebugging();
		static bool IsDebugging();
		static void StopDebugging();

		static physx::PxPvd* GetDebugger();
	};
}
