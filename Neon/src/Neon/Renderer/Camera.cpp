#include "neopch.h"

#include "Neon/Renderer/Camera.h"

namespace Neon
{
	glm::mat4 Camera::GetViewMatrix() const
	{
		Transform transform = GetTransform();
		return glm::inverse(glm::translate(glm::mat4(1.0f), transform.Translation) * glm::toMat4(transform.Rotation));
	}

} // namespace Neon
