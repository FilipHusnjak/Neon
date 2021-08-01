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

	SharedRef<PhysicsActor> PhysicsScene::CreateActor(Entity entity)
	{
		SharedRef<PhysicsActor> actor = InternalCreateActor(entity);
		m_Actors.push_back(actor);
		return actor;
	}

	void PhysicsScene::RemoveActor(SharedRef<PhysicsActor> actor)
	{
		InternalRemoveActor(actor);

		for (auto it = m_Actors.begin(); it != m_Actors.end(); it++)
		{
			if ((*it)->GetEntity() == actor->GetEntity())
			{
				m_Actors.erase(it);
				break;
			}
		}
	}

	void PhysicsScene::Destroy()
	{
		for (auto& actor : m_Actors)
		{
			RemoveActor(actor);
		}
	}

} // namespace Neon
