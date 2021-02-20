#pragma once

#include "Neon/Core/UUID.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Renderer/RendererContext.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Neon
{
	struct IDComponent
	{
		UUID ID = 0;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
		TagComponent(const std::string& tag)
			: Tag(tag)
		{
		}

		operator std::string &()
		{
			return Tag;
		}
		operator const std::string &() const
		{
			return Tag;
		}
	};

	struct TransformComponent
	{
		glm::vec3 Translation;
		glm::vec3 Rotation;
		glm::vec3 Scale;

		TransformComponent(const glm::vec3& translation = {}, glm::vec3 rotation = glm::vec3(0.f),
						   const glm::vec3 scale = {1.f, 1.f, 1.f})
			: Translation(translation)
			, Rotation(rotation)
			, Scale(scale)
		{
		}
		TransformComponent(const TransformComponent& other) = default;

		operator glm::mat4()
		{
			return glm::translate(glm::mat4(1.f), Translation) * glm::eulerAngleXYZ(Rotation.x, Rotation.y, Rotation.z) *
				   glm::scale(glm::mat4(1.f), glm::abs(Scale));
		}
	};

	struct MeshComponent
	{
		SharedRef<Neon::Mesh> Mesh;

		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;
		MeshComponent(const SharedRef<Neon::Mesh>& mesh)
			: Mesh(mesh)
		{
		}
		~MeshComponent()
		{
			RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(Mesh));
		}

		operator SharedRef<Neon::Mesh>()
		{
			return Mesh;
		}
	};	

	struct LightComponent
	{
		LightComponent() = default;
		LightComponent(const LightComponent& other) = default;
		LightComponent(const glm::vec4& direction)
			: Direction(direction)
		{
		}

		float Strength = 1.f;
		glm::vec4 Direction = {0.f, 0.f, 0.f, 0.f};
		glm::vec4 Radiance = {1.f, 1.f, 1.f, 1.f};
	};
} // namespace Neon
