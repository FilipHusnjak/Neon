#include "neopch.h"

#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{

	Transform::Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
		: Translation(translation)
		, Rotation(rotation)
		, Scale(scale)
	{
	}

	glm::mat4 Transform::GetMatrix() const
	{
		return glm::translate(glm::mat4(1.f), Translation) * glm::toMat4(glm::quat(Rotation)) * glm::scale(glm::mat4(1.f), glm::abs(Scale));
	}
} // namespace Neon
