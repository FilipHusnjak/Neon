#pragma once

#include <glm/glm.hpp>

namespace Neon
{
	class LightComponent
	{
	public:
		LightComponent() = default;
		LightComponent(const LightComponent& other) = default;
		LightComponent(const glm::vec4& direction);

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
