#pragma once

#include "Neon/Scene/Components/ActorComponent.h"

#include <glm/glm.hpp>

namespace Neon
{
	class LightComponent : public ActorComponent
	{
	public:
		LightComponent() = default;
		LightComponent(Actor* owner, const glm::vec4& direction);
		LightComponent(const LightComponent& other) = default;
		virtual ~LightComponent() = default;

		float GetStrength() const
		{
			return m_Strength;
		}
		const glm::vec4& GetDirection() const
		{
			return m_Direction;
		}
		const glm::vec4& GetRadiance() const
		{
			return m_Radiance;
		}

	private:
		float m_Strength = 1.f;
		glm::vec4 m_Direction = {0.f, 0.f, 0.f, 0.f};
		glm::vec4 m_Radiance = {1.f, 1.f, 1.f, 1.f};
	};
} // namespace Neon
