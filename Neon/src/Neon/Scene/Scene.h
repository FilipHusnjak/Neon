#pragma once

#include "Neon/Core/UUID.h"
#include "Neon/Editor/EditorCamera.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Neon
{
	class Entity;
	class Material;

	using EntityMap = std::unordered_map<UUID, Entity>;

	class Scene : public RefCounted
	{
	public:
		Scene(const std::string debugName = "Scene");
		~Scene();

		void Init();

		void OnUpdate(float deltaSeconds);
		void OnRenderEditor(float deltaSeconds, const EditorCamera& editorCamera);
		void OnEvent(Event& e);

		void SetViewportSize(uint32 width, uint32 height);

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithID(UUID uuid, const std::string& name = "", bool runtimeMap = false);
		Entity CreateMesh(const std::string& path, const std::string& name = "");
		void DestroyEntity(Entity entity);

		template<typename T>
		auto GetAllEntitiesWithComponent()
		{
			return m_Registry.view<T>();
		}

		template<typename T>
		T& GetEntityComponent(entt::entity entity)
		{
			return m_Registry.get<T>(entity);
		}

		Entity FindEntityByTag(const std::string& tag);

		const EntityMap& GetEntityMap() const
		{
			return m_EntityIDMap;
		}

		UUID GetUUID() const
		{
			return m_SceneID;
		}

		static SharedRef<Scene> GetScene(UUID uuid);

		void SetSelectedEntity(entt::entity entity)
		{
			m_SelectedEntity = entity;
		}

	private:
		UUID m_SceneID;
		entt::entity m_SceneEntity;
		entt::registry m_Registry;

		std::string m_DebugName;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		EntityMap m_EntityIDMap;

		SharedRef<Material> m_SkyboxMaterial;

		entt::entity m_SelectedEntity;

		float m_SkyboxLod = 1.0f;

		friend class Entity;
		friend class SceneRenderer;
	};
} // namespace Neon
