#include "neopch.h"

#include "PerspectiveCamera.h"

namespace Neon
{
	PerspectiveCamera::PerspectiveCamera(float fovY, float aspect, float zNear, float zFar)
		: m_ProjectionMatrix(glm::perspective(fovY, aspect, zNear, zFar))
	{
		m_ProjectionMatrix[1][1] *= -1;
		RecalculateViewMatrix();
	}

	void PerspectiveCamera::SetProjection(float fovY, float aspect, float zNear, float zFar)
	{
		m_ProjectionMatrix = glm::perspective(fovY, aspect, zNear, zFar);
		m_ProjectionMatrix[1][1] *= -1;
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
	}
}
