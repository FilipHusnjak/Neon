#pragma once

#include "Neon/Scene/Components/ActorComponent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
	class CameraComponent : public ActorComponent
	{
	public:
		CameraComponent(Actor* owner);
		virtual ~CameraComponent() = default;

		void SetPositionOffset(const glm::vec3& positionOffset)
		{
			m_PositionOffset = positionOffset;
		}
		void SetRotationOffset(const glm::quat& rotationOffset)
		{
			m_RotationOffset = rotationOffset;
		}
		void SetAspectRatio(float aspectRatio)
		{
			m_AspectRatio = aspectRatio;
		}
		void SetFov(float fov)
		{
			m_Fov = fov;
		}

		glm::vec3 GetPosition() const;
		glm::quat GetRotation() const;
		Transform GetTransform() const;

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix() const;
		glm::mat4 GetViewProjectionMatrix() const
		{
			return GetProjectionMatrix() * GetViewMatrix();
		}

	private:
		glm::vec3 m_PositionOffset = glm::vec3();
		glm::quat m_RotationOffset = glm::quat(1.f, 0.f, 0.f, 0.f);

		float m_AspectRatio = 1.6f;
		float m_Fov = 45.f;
	};
} // namespace Neon
