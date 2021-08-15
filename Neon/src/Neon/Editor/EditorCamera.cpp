#include "neopch.h"

#include "EditorCamera.h"
#include "Neon/Core/Input.h"

#include <glfw/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define M_PI 3.14159f

namespace Neon
{
	EditorCamera::EditorCamera()
	{
		m_FocalPoint = glm::vec3(0.f, 5.f, 0.f);

		m_Distance = 100.f;

		m_Yaw = 3.0f * M_PI / 4.0f;
		m_Pitch = M_PI / 8.0f;
	}

	void EditorCamera::Focus()
	{
	}

	void EditorCamera::Tick(float deltaSeconds)
	{
		glm::vec2 delta = Input::GetMouseDelta();
		if (Input::IsKeyPressed(GLFW_KEY_LEFT_ALT))
		{
			if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
			{
				MousePan(delta);
			}
			else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
			{
				MouseRotate(delta);
			}
			else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
			{
				MouseZoom(delta.y);
			}
		}
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) { return OnMouseScroll(e); });
	}

	glm::vec3 EditorCamera::GetPosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetRotation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	Transform EditorCamera::GetTransform() const
	{
		return {GetPosition(), GetRotation(), glm::vec3(1.f)};
	}

	glm::mat4 EditorCamera::GetProjectionMatrix() const
	{
		glm::mat4 projectionMatrix =
			glm::perspective(glm::radians(m_Fov), static_cast<float>(m_ViewportWidth) / m_ViewportHeight, 0.1f, 10000.f);
		// TODO: Only for Vulkan
		projectionMatrix[1][1] *= -1;
		return projectionMatrix;
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetRotation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetRotation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetRotation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return {xFactor, yFactor};
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

} // namespace Neon
