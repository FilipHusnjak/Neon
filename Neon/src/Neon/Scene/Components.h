#pragma once

#include "Neon/Renderer/Mesh.h"
#include "Neon/Core/UUID.h"

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
		glm::mat4 Transform;

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(const glm::mat4& transform)
			: Transform(transform)
		{
		}

		operator glm::mat4 &()
		{
			return Transform;
		}
		operator const glm::mat4 &() const
		{
			return Transform;
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

		float Strength = 3.f;
		glm::vec4 Direction = {0.f, 0.f, 0.f, 0.f};
		glm::vec4 Radiance = {1.f, 1.f, 1.f, 1.f};
	};
} // namespace Neon
