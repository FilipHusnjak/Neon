#pragma once

#include "Scene.h"

namespace Neon
{
	class Actor : public RefCounted
	{
	public:
		Actor() = default;
		Actor(entt::entity handle, Scene* scene, const std::string& tag, UUID id, glm::vec3 translation = glm::vec3(),
			  glm::vec3 rotation = glm::vec3(), glm::vec3 scale = glm::vec3(1.f));
		Actor(const Actor& other) = delete;
		Actor(const Actor&& other) = delete;
		Actor& operator=(const Actor& other) = delete;
		Actor& operator=(const Actor&& other) = delete;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			assert(!HasComponent<T>());
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			assert(HasComponent<T>());
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			assert(HasComponent<T>());
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		entt::entity GetHandle() const
		{
			return m_EntityHandle;
		}

		operator bool() const
		{
			return m_EntityHandle != entt::null && m_Scene;
		}

		bool operator==(Actor other) const
		{
			return m_EntityHandle == other.m_EntityHandle;
		}

		const std::string& GetTag() const
		{
			return m_Tag;
		}

		const UUID GetID() const
		{
			return m_ID;
		}

		const glm::vec3& GetTranslation() const
		{
			return m_Translation;
		}
		const glm::vec3& GetRotation() const
		{
			return m_Rotation;
		}
		const glm::vec3& GetScale() const
		{
			return m_Scale;
		}

		glm::vec3& GetTranslation()
		{
			return m_Translation;
		}
		glm::vec3& GetRotation()
		{
			return m_Rotation;
		}
		glm::vec3& GetScale()
		{
			return m_Scale;
		}

		const glm::mat4 GetTransformMat();

	private:
		entt::entity m_EntityHandle{entt::null};
		Scene* m_Scene = nullptr;

		std::string m_Tag = std::string();
		UUID m_ID = 0;

		glm::vec3 m_Translation;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;

		friend class Scene;
	};
} // namespace Neon
