#pragma once

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Physics/PhysicsSettings.h"

namespace Neon
{
	class PhysicsScene : public RefCounted
	{
	public:
		PhysicsScene(const PhysicsSettings& settings);
		virtual ~PhysicsScene();

		virtual void Tick(float deltaSeconds) = 0;

		uint32 GetNumSubsteps(float deltaSeconds);

		SharedRef<PhysicsBody> AddPhysicsBody(PhysicsBodyType physicsBodyType, const Transform& transform);
		void RemovePhysicsBody(SharedRef<PhysicsBody> physicsBody);

		virtual void Destroy();

	protected:
		virtual SharedRef<PhysicsBody> InternalAddPhysicsBody(PhysicsBodyType physicsBodyType, const Transform& transform) = 0;
		virtual void InternalRemovePhysicsBody(SharedRef<PhysicsBody> physicsBody) = 0;

	protected:
		std::vector<SharedRef<PhysicsBody>> m_PhysicsBodies;

		float m_SubStepSize;
		float m_Accumulator = 0.0f;
		const uint32_t c_MaxSubSteps = 8;
	};
} // namespace Neon
