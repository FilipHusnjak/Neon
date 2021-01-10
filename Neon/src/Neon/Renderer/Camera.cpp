#include "Camera.h"
#include "neopch.h"

namespace Neon
{
	Camera::Camera(const glm::mat4& projectionMatrix)
		: m_ProjectionMatrix(projectionMatrix)
	{
	}
} // namespace Neon
