#pragma once

#include "Neon/Math/Math.h"
#include "Neon/Scene/Components/ActorComponent.h"
#include "Neon/Scene/Scene.h"

namespace Neon
{
	class Actor : public RefCounted
	{
	public:
		Actor(Scene* scene, const std::string& tag, UUID id, Transform transform = Transform());
		Actor(const Actor& other) = delete;
		Actor(const Actor&& other) = delete;
		Actor& operator=(const Actor& other) = delete;
		Actor& operator=(const Actor&& other) = delete;
		virtual ~Actor() = default;

		virtual void Initialize()
		{
		}

		template<typename T, typename... Args>
		SharedRef<T> AddRootComponent(Args&&... args)
		{
			if (m_RootComponent)
			{
				RemoveComponent(m_RootComponent);
			}
			m_RootComponent = SharedRef<T>::Create(std::forward<Args>(args)...);
			m_ActorComponents.push_back(m_RootComponent);
			return m_RootComponent.As<T>();
		}

		template<typename T, typename... Args>
		SharedRef<T> AddComponent(Args&&... args)
		{
			return m_ActorComponents.emplace_back(SharedRef<T>::Create(std::forward<Args>(args)...)).As<T>();
		}

		template<typename T>
		SharedRef<T> GetRootComponent()
		{
			SharedRef<T> component = m_RootComponent.As<T>();
			NEO_CORE_ASSERT(component);
			return component;
		}

		std::vector<SharedRef<ActorComponent>>& GetActorComponents()
		{
			return m_ActorComponents;
		}
		const std::vector<SharedRef<ActorComponent>>& GetActorComponents() const
		{
			return m_ActorComponents;
		}

		void RemoveComponent(const SharedRef<ActorComponent>& component);

		const std::string& GetTag() const
		{
			return m_Tag;
		}

		const UUID GetID() const
		{
			return m_ID;
		}

		const Transform& GetTransform() const
		{
			return m_ActorTransform;
		}
		const glm::vec3& GetTranslation() const
		{
			return m_ActorTransform.Translation;
		}
		const glm::quat& GetRotation() const
		{
			return m_ActorTransform.Rotation;
		}
		const glm::vec3& GetScale() const
		{
			return m_ActorTransform.Scale;
		}

		Transform& GetTransform()
		{
			return m_ActorTransform;
		}
		glm::vec3& GetTranslation()
		{
			return m_ActorTransform.Translation;
		}
		glm::quat& GetRotation()
		{
			return m_ActorTransform.Rotation;
		}
		glm::vec3& GetScale()
		{
			return m_ActorTransform.Scale;
		}

		virtual void Tick(float deltaSeconds);

		void SetTransform(const Transform& transform);
		void SetTranslation(const glm::vec3& translation);
		void SetRotation(const glm::quat& rotation);
		void SetScale(const glm::vec3& scale);

	private:
		Scene* m_Scene = nullptr;

		std::string m_Tag = std::string();
		UUID m_ID = 0;

		Transform m_ActorTransform;

		SharedRef<ActorComponent> m_RootComponent = nullptr;
		std::vector<SharedRef<ActorComponent>> m_ActorComponents;

		friend class Scene;
	};
} // namespace Neon
