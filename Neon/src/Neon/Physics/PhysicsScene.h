#pragma once

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Physics/PhysicsConstraint.h"
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

		virtual SharedRef<PhysicsConstraint> AddPhysicsConstraint(const SharedRef<PhysicsBody>& body0,
																  const SharedRef<PhysicsBody>& body1);
		virtual void RemovePhysicsConstraint(SharedRef<PhysicsConstraint>& physicsConstraint);

		virtual SharedRef<PhysicsBody> AddPhysicsBody(PhysicsBodyType physicsBodyType, const Transform& transform,
													  const SharedRef<PhysicsMaterial>& material);
		virtual void RemovePhysicsBody(SharedRef<PhysicsBody>& physicsBody);

		virtual void Destroy();

	protected:
		std::vector<SharedRef<PhysicsConstraint>> m_PhysicsConstraints;
		std::vector<SharedRef<PhysicsBody>> m_PhysicsBodies;

		float m_SubStepSize;
		float m_Accumulator = 0.0f;
		const uint32_t c_MaxSubSteps = 8;
	};
} // namespace Neon
