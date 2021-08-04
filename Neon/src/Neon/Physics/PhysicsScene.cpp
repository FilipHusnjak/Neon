#include "neopch.h"

#include "PhysicsScene.h"

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

	SharedRef<PhysicsBody> PhysicsScene::AddPhysicsBody(PhysicsBodyType physicsBodyType, const Transform& transform)
	{
		SharedRef<PhysicsBody> physicsBody = InternalAddPhysicsBody(physicsBodyType, transform);
		m_PhysicsBodies.emplace_back(physicsBody);
		return physicsBody;
	}

	void PhysicsScene::RemovePhysicsBody(SharedRef<PhysicsBody> physicsBody)
	{
		InternalRemovePhysicsBody(physicsBody);

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
		for (auto it = m_PhysicsBodies.begin(); it != m_PhysicsBodies.end();)
		{
			InternalRemovePhysicsBody(*it);
			it = m_PhysicsBodies.erase(it);
		}
	}

} // namespace Neon
