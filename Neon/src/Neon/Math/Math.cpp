#include "neopch.h"

#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

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
		return glm::translate(glm::mat4(1.f), Translation) * glm::eulerAngleXYZ(Rotation.x, Rotation.y, Rotation.z) *
			   glm::scale(glm::mat4(1.f), glm::abs(Scale));
	}
} // namespace Neon
