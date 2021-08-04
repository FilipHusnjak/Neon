#include "neopch.h"

#include "LightComponent.h"
#include "Neon/Renderer/SceneRenderer.h"

namespace Neon
{
	LightComponent::LightComponent(Actor* owner, const glm::vec4& direction)
		: ActorComponent(owner)
	{
		m_Light.Direction = direction;
	}

	void LightComponent::TickComponent(float deltaSeconds)
	{
		ActorComponent::TickComponent(deltaSeconds);

		SceneRenderer::SubmitLight(m_Light);
	}

} // namespace Neon
