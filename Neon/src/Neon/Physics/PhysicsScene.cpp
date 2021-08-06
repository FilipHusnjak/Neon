#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsBody.h"
#include "Neon/Physics/PhysX/PhysXPhysicsConstraint.h"
#include "Neon/Physics/Physics.h"
#include "Neon/Physics/PhysicsScene.h"

namespace Neon
{
	PhysicsScene::PhysicsScene(const PhysicsSettings& settings)
		: m_SubStepSize(settings.FixedTimestep)
	{
	}

	PhysicsScene::~PhysicsScene()
	{
	}

	uint32 PhysicsScene::GetNumSubsteps(float deltaSeconds)
	{
		if (m_Accumulator > m_SubStepSize)
		{
			m_Accumulator = 0.0f;
		}

		m_Accumulator += deltaSeconds;
		if (m_Accumulator < m_SubStepSize)
		{
			return 0;
		}

		uint32 substepCount = glm::min(static_cast<uint32>(m_Accumulator / m_SubStepSize), c_MaxSubSteps);
		m_Accumulator -= (float)substepCount * m_SubStepSize;
		return substepCount;
	}

	SharedRef<PhysicsConstraint> PhysicsScene::AddPhysicsConstraint(const SharedRef<PhysicsBody>& body0,
																	const SharedRef<PhysicsBody>& body1)
	{
		switch (Physics::GetCurrentEngine())
		{
			case PhysicsEngine::PhysX:
			{
				return m_PhysicsConstraints.emplace_back(SharedRef<PhysXPhysicsConstraint>::Create(body0, body1));
			}
			default:
			{
				NEO_CORE_ASSERT(false, "Unknown physics engine!");
				return nullptr;
			}
		}
	}

	void PhysicsScene::RemovePhysicsConstraint(SharedRef<PhysicsConstraint>& physicsConstraint)
	{
		physicsConstraint->Destroy();
		for (auto it = m_PhysicsConstraints.begin(); it != m_PhysicsConstraints.end(); it++)
		{
			if (it->Ptr() == physicsConstraint.Ptr())
			{
				m_PhysicsConstraints.erase(it);
				break;
			}
		}
	}

	SharedRef<PhysicsBody> PhysicsScene::AddPhysicsBody(PhysicsBodyType physicsBodyType, const Transform& transform)
	{
		switch (Physics::GetCurrentEngine())
		{
			case PhysicsEngine::PhysX:
			{
				return m_PhysicsBodies.emplace_back(SharedRef<PhysXPhysicsBody>::Create(physicsBodyType, transform));
			}
			default:
			{
				NEO_CORE_ASSERT(false, "Unknown physics engine!");
				return nullptr;
			}
		}
	}

	void PhysicsScene::RemovePhysicsBody(SharedRef<PhysicsBody>& physicsBody)
	{
		physicsBody->Destroy();
		for (auto it = m_PhysicsBodies.begin(); it != m_PhysicsBodies.end(); it++)
		{
			if (it->Ptr() == physicsBody.Ptr())
			{
				m_PhysicsBodies.erase(it);
				break;
			}
		}
	}

	void PhysicsScene::Destroy()
	{
		for (auto& constraint : m_PhysicsConstraints)
		{
			constraint->Destroy();
		}
		m_PhysicsConstraints.clear();

		for (auto& body : m_PhysicsBodies)
		{
			body->Destroy();
		}
		m_PhysicsBodies.clear();
	}

} // namespace Neon
