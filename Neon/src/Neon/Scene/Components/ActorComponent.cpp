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
		NEO_CORE_ASSERT(m_Owner);
	}

	void ActorComponent::RenderGui()
	{
	}

} // namespace Neon
