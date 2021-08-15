#include "neopch.h"

#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
	static glm::vec3 GetSafeReciprocal(const glm::vec3& v)
	{
		glm::vec3 result;

		if (glm::abs(v.x) < SMALL_NUMBER)
		{
			result.x = 0.f;
		}
		else
		{
			result.x = 1.f / v.x;
		}
		if (glm::abs(v.y) < SMALL_NUMBER)
		{
			result.y = 0.f;
		}
		else
		{
			result.y = 1.f / v.y;
		}
		if (glm::abs(v.z) < SMALL_NUMBER)
		{
			result.z = 0.f;
		}
		else
		{
			result.z = 1.f / v.z;
		}
		return result;
	}

	Transform::Transform(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale)
		: Translation(translation)
		, Rotation(rotation)
		, Scale(scale)
	{
	}

	Transform Transform::Inverse() const
	{
		glm::vec3 invScale = GetSafeReciprocal(Scale);
		glm::quat invRotation = glm::inverse(Rotation);
		glm::vec3 invTranslation = -(invRotation * (invScale * Translation));
		return Transform(invTranslation, invRotation, invScale);
	}

	glm::mat4 Transform::GetMatrix() const
	{
		return glm::translate(glm::mat4(1.f), Translation) * glm::toMat4(Rotation) * glm::scale(glm::mat4(1.f), glm::abs(Scale));
	}

	glm::vec3 Transform::TransformVector(const glm::vec3& v) const
	{
		return glm::rotate(Rotation, v);
	}

	Transform Transform::operator*(const Transform& other) const
	{
		Transform result;
		result.Rotation = Rotation * other.Rotation;
		result.Scale = Scale * other.Scale;
		result.Translation = Rotation * (Scale * other.Translation) + Translation;
		return result;
	}

} // namespace Neon
