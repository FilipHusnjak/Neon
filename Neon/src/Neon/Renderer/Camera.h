#pragma once

#include <glm/glm.hpp>

namespace Neon
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projectionMatrix);
		virtual ~Camera() = default;

		const glm::mat4& GetProjectionMatrix() const
		{
			return m_ProjectionMatrix;
		}
		void SetProjectionMatrix(const glm::mat4& projectionMatrix)
		{
			m_ProjectionMatrix = projectionMatrix;
			// TODO: Only for Vulkan
			m_ProjectionMatrix[1][1] *= -1;
		}

		const glm::vec3& GetPosition() const
		{
			return m_Position;
		}

		const glm::mat4 GetViewMatrix() const
		{
			return m_ViewMatrix;
		}
		glm::mat4 GetViewProjection() const
		{
			return m_ProjectionMatrix * m_ViewMatrix;
		}

		float GetExposure() const
		{
			return m_Exposure;
		}
		float& GetExposure()
		{
			return m_Exposure;
		}

	protected:
		glm::vec3 m_Position;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

		float m_Exposure = 0.8f;
	};
} // namespace Neon
