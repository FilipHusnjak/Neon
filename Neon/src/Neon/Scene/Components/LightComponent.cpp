#include "neopch.h"

#include "LightComponent.h"

namespace Neon
{
	LightComponent::LightComponent(Actor* owner, const glm::vec4& direction)
		: ActorComponent(owner)
		, m_Direction(direction)
	{
	}

} // namespace Neon
