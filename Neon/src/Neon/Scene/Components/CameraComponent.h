#pragma once

#include "Neon/Scene/Components/ActorComponent.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
	class CameraComponent : public ActorComponent, public Camera
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
		virtual void SetViewportSize(uint32 width, uint32 height) override
		{
			m_Width = width;
			m_Height = height;
		}
		virtual void SetFov(float fov) override
		{
			m_Fov = fov;
		}

		virtual glm::vec3 GetPosition() const override;
		virtual glm::quat GetRotation() const override;
		virtual Transform GetTransform() const override;

		virtual glm::mat4 GetProjectionMatrix() const override;		

	private:
		glm::vec3 m_PositionOffset = glm::vec3();
		glm::quat m_RotationOffset = glm::quat(1.f, 0.f, 0.f, 0.f);

		uint32 m_Width = 1920;
		uint32 m_Height = 1080;
		float m_Fov = 45.f;
	};
} // namespace Neon
