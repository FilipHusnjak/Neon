#include "neopch.h"

#include "Neon/Scene/Actor.h"

namespace Neon
{
	Actor::Actor(Scene* scene, const std::string& tag, UUID id, Transform transform)
		: m_Scene(scene)
		, m_Tag(tag)
		, m_ID(id)
		, m_ActorTransform(transform)
	{
	}

	void Actor::RemoveComponent(const SharedRef<ActorComponent>& component)
	{
		if (m_RootComponent == component)
		{
			m_RootComponent.Reset();
		}

		for (auto it = m_ActorComponents.begin(); it != m_ActorComponents.end();)
		{
			if (it->Ptr() == component.Ptr())
			{
				it = m_ActorComponents.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void Actor::Tick(float deltaSeconds)
	{
		for (auto& component : m_ActorComponents)
		{
			NEO_CORE_ASSERT(component);
			component->TickComponent(deltaSeconds);
		}
	}

	void Actor::SetTranslation(glm::vec3 translation)
	{
		m_ActorTransform.Translation = translation;
	}
	void Actor::SetRotation(glm::vec3 rotation)
	{
		m_ActorTransform.Rotation = rotation;
	}
	void Actor::SetScale(glm::vec3 scale)
	{
		m_ActorTransform.Scale = scale;
	}

} // namespace Neon
