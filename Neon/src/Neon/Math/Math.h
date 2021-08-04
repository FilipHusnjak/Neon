#pragma once

#include <glm/glm.hpp>

namespace Neon
{
	struct Transform
	{
		Transform() = default;
		Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);

		glm::vec3 Translation = glm::vec3();
		glm::vec3 Rotation = glm::vec3();
		glm::vec3 Scale = glm::vec3(1.f);

		glm::mat4 GetMatrix() const;
	};
} // namespace Neon
