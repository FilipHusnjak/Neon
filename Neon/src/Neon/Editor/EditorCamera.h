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

		void Focus();
		void Tick(float deltaSeconds);
		void OnEvent(Event& e);

		virtual void SetViewportSize(uint32 width, uint32 height) override
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
		}

		virtual void SetFov(float fov) override
		{
			m_Fov = fov;
		}

		virtual glm::vec3 GetPosition() const override;
		virtual glm::quat GetRotation() const override;
		virtual Transform GetTransform() const override;

		virtual glm::mat4 GetProjectionMatrix() const;

		float GetDistance() const
		{
			return m_Distance;
		}
		void SetDistance(float distance)
		{
			m_Distance = distance;
		}

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;

		float GetPitch() const
		{
			return m_Pitch;
		}
		float GetYaw() const
		{
			return m_Yaw;
		}

	private:
		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		glm::vec3 m_FocalPoint;

		float m_Distance;
		float m_Pitch;
		float m_Yaw;

		uint32 m_ViewportWidth = 1920;
		uint32 m_ViewportHeight = 1080;

		float m_Fov = 45.f;
	};
} // namespace Neon
