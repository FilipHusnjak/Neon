#include "neopch.h"

#include "ActorComponent.h"

namespace Neon
{
	ActorComponent::ActorComponent(Actor* owner)
		: m_Owner(owner)
	{
	}

	void ActorComponent::TickComponent(float deltaSeconds)
	{
	}
} // namespace Neon
