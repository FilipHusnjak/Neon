#pragma once

#include "Neon/Math/Math.h"

#include <glm/glm.hpp>

namespace Neon
{
	class Camera
	{
	public:
		Camera() = default;
		virtual ~Camera() = default;

		virtual void SetViewportSize(uint32 width, uint32 height) = 0;

		virtual void SetFov(float fov) = 0;

		virtual glm::vec3 GetPosition() const = 0;
		virtual glm::quat GetRotation() const = 0;
		virtual Transform GetTransform() const = 0;

		glm::mat4 GetViewMatrix() const;
		virtual glm::mat4 GetProjectionMatrix() const = 0;
		glm::mat4 GetViewProjectionMatrix() const
		{
			return GetProjectionMatrix() * GetViewMatrix();
		}
	};
} // namespace Neon
