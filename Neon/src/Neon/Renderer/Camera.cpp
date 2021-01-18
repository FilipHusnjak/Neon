#include "neopch.h"

#include "Camera.h"

namespace Neon
{
	Camera::Camera(const glm::mat4& projectionMatrix)
		: m_ProjectionMatrix(projectionMatrix)
	{
	}
} // namespace Neon
