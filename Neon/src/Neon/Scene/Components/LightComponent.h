#pragma once

#include "Neon/Scene/Components/ActorComponent.h"

#include <glm/glm.hpp>

namespace Neon
{
	struct Light
	{
		float Strength = 1.f;
		glm::vec4 Direction = {0.f, 0.f, 0.f, 0.f};
		glm::vec4 Radiance = {1.f, 1.f, 1.f, 1.f};
	};

	class LightComponent : public ActorComponent
	{
	public:
		LightComponent(Actor* owner, const glm::vec4& direction = glm::vec4(1.f));
		LightComponent(const LightComponent& other) = default;
		virtual ~LightComponent() = default;

		virtual void TickComponent(float deltaSeconds) override;

		float GetStrength() const
		{
			return m_Light.Strength;
		}
		const glm::vec4& GetDirection() const
		{
			return m_Light.Direction;
		}
		const glm::vec4& GetRadiance() const
		{
			return m_Light.Radiance;
		}

	private:
		Light m_Light;
	};
} // namespace Neon
