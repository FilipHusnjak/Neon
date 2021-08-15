#include "neopch.h"

#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Components/CameraComponent.h"

namespace Neon
{
	CameraComponent::CameraComponent(Actor* owner)
		: ActorComponent(owner)
	{
	}

	glm::vec3 CameraComponent::GetPosition() const
	{
		return GetTransform().Translation;
	}

	glm::quat CameraComponent::GetRotation() const
	{
		return GetTransform().Rotation;
	}

	Transform CameraComponent::GetTransform() const
	{
		Transform localTransform = Transform(m_PositionOffset, m_RotationOffset, glm::vec3(1.f));
		Transform globalTransform = m_Owner->GetTransform();
		return globalTransform * localTransform;
	}

	glm::mat4 CameraComponent::GetViewMatrix() const
	{
		Transform transform = GetTransform();
		return glm::inverse(glm::translate(glm::mat4(1.0f), transform.Translation) * glm::toMat4(transform.Rotation));
	}

	glm::mat4 CameraComponent::GetProjectionMatrix() const
	{
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, 0.1f, 10000.f);
		// TODO: Only for Vulkan
		projectionMatrix[1][1] *= -1;
		return projectionMatrix;
	}

} // namespace Neon