#include "neopch.h"

#include "Neon/Physics/PhysX\PhysXPhysics.h"
#include "Neon/Physics/Physics.h"

namespace Neon
{
	PhysicsSettings Physics::s_Settings;
	SharedRef<PhysicsScene> Physics::s_Scene;

	static PhysicsEngine s_PE = PhysicsEngine::PhysX;

	static SharedRef<Physics> s_Physics;

	void* Physics::GetPhysicsSDK()
	{
		NEO_CORE_ASSERT(s_Physics, "Physics not initialized!");

		return s_Physics->InternalGetPhysicsSDK();
	}

	void Physics::TickPhysics(float deltaSeconds)
	{
		NEO_CORE_ASSERT(s_Scene);

		s_Scene->Tick(deltaSeconds);
	}

	void Physics::Initialize()
	{
		NEO_CORE_ASSERT(!s_Physics, "Physics already initialized!");

		if (s_PE == PhysicsEngine::PhysX)
		{
			s_Physics = SharedRef<PhysXPhysics>::Create();
		}
		else
		{
			NEO_CORE_ASSERT(false, "Uknown physics engine selected!");
		}

		s_Physics->InternalInitialize();
	}

	void Physics::Shutdown()
	{
		NEO_CORE_ASSERT(s_Physics, "Physics not initialized!");

		s_Physics->InternalShutdown();
	}

	void Physics::CreateScene()
	{
		NEO_CORE_ASSERT(s_Physics, "Physics not initialized!");

		s_Physics->InternalCreateScene();
	}

	void Physics::DestroyScene()
	{
		NEO_CORE_ASSERT(s_Physics, "Physics not initialized!");

		s_Physics->InternalDestroyScene();

		s_Scene->Destroy();
	}

	SharedRef<PhysicsScene> Physics::GetCurrentScene()
	{
		NEO_CORE_ASSERT(s_Scene, "Scene not created!");

		return s_Scene;
	}

	const SharedRef<Physics> Physics::Get()
	{
		NEO_CORE_ASSERT(s_Physics, "Physics not initialized!");

		return s_Physics;
	}

} // namespace Neon
