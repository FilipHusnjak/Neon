#pragma once

#include "Neon/Core/Event/MouseEvent.h"
#include "Neon/Renderer/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Neon
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera();
		EditorCamera(const glm::mat4& projectionMatrix);

		void Focus();
		void OnUpdate(float deltaSeconds);
		void OnEvent(Event& e);

		float GetDistance() const
		{
			return m_Distance;
		}
		void SetDistance(float distance)
		{
			m_Distance = distance;
		}

		void SetViewportSize(uint32 width, uint32 height)
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
		}

		glm::vec3 GetUpDirection();
		glm::vec3 GetRightDirection();
		glm::vec3 GetForwardDirection();

		glm::quat GetOrientation() const;

		float GetPitch() const
		{
			return m_Pitch;
		}
		float GetYaw() const
		{
			return m_Yaw;
		}

	private:
		void UpdateCameraView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition();

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		glm::vec3 m_FocalPoint;

		glm::vec2 m_InitialMousePosition;
		glm::vec3 m_InitialFocalPoint;
		glm::vec3 m_InitialRotation;

		float m_Distance;
		float m_Pitch;
		float m_Yaw;

		uint32 m_ViewportWidth = 1920;
		uint32 m_ViewportHeight = 1080;
	};
} // namespace Neon
