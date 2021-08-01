#pragma once

#include "Neon/Physics/PhysicsActor.h"
#include "Neon/Physics/PhysicsSettings.h"

namespace Neon
{
	class PhysicsScene : public RefCounted
	{
	public:
		PhysicsScene(const PhysicsSettings& settings);
		virtual ~PhysicsScene();

		SharedRef<PhysicsActor> CreateActor(Entity entity);
		void RemoveActor(SharedRef<PhysicsActor> actor);

		virtual void Destroy();

	protected:
		virtual SharedRef<PhysicsActor> InternalCreateActor(Entity entity) = 0;
		virtual void InternalRemoveActor(SharedRef<PhysicsActor> actor) = 0;

	protected:
		std::vector<SharedRef<PhysicsActor>> m_Actors;

		float m_SubStepSize;
		float m_Accumulator = 0.0f;
		uint32_t m_NumSubSteps = 0;
		const uint32_t c_MaxSubSteps = 8;
	};
} // namespace Neon
