#pragma once

#include "Neon/Physics/PhysicsScene.h"

namespace Neon
{
	enum class PhysicsEngine
	{
		None,
		PhysX
	};

	enum class ForceMode : uint8_t
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	class Physics : public RefCounted
	{
	public:
		static void* GetPhysicsSDK();

		static void TickPhysics(float deltaSeconds);

		static void Initialize();
		static void Shutdown();

		static void CreateScene();
		static void DestroyScene();

		static SharedRef<PhysicsScene> GetCurrentScene();

		static const SharedRef<Physics> Get();

	public:
		virtual ~Physics() = default;

	protected:
		Physics() = default;

		virtual void* InternalGetPhysicsSDK() const = 0;

		virtual void InternalInitialize() = 0;
		virtual void InternalShutdown() = 0;

		virtual void InternalCreateScene() = 0;
		virtual void InternalDestroyScene() = 0;

	protected:
		static PhysicsSettings s_Settings;
		static SharedRef<PhysicsScene> s_Scene;
	};
} // namespace Neon
