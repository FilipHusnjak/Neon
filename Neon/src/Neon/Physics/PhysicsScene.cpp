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

	SharedRef<PhysicsBody> PhysicsScene::AddPhysicsBody()
	{
		SharedRef<PhysicsBody> physicsBody = InternalAddPhysicsBody();
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
		for (auto& physicsBody : m_PhysicsBodies)
		{
			RemovePhysicsBody(physicsBody);
		}
	}

} // namespace Neon
