#pragma once

#include "Scene.h"

namespace Neon
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene)
			: m_EntityHandle(handle)
			, m_Scene(scene)
		{
		}
		Entity(const Entity& other) = default;

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

		explicit operator bool() const
		{
			return m_EntityHandle != entt::null;
		}

		bool operator==(Entity other) const
		{
			return m_EntityHandle == other.m_EntityHandle;
		}

	private:
		entt::entity m_EntityHandle{entt::null};
		Scene* m_Scene = nullptr;

		friend class Scene;
	};
} // namespace Neon
