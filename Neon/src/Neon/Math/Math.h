#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
	struct Transform
	{
		Transform() = default;
		Transform(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale);

		Transform Inverse() const;

		glm::vec3 Translation = glm::vec3();
		glm::quat Rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
		glm::vec3 Scale = glm::vec3(1.f);

		glm::mat4 GetMatrix() const;

		glm::vec3 TransformVector(const glm::vec3& v) const;

		Transform operator*(const Transform& other) const;
	};
} // namespace Neon
