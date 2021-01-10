#include "neopch.h"

#include "Neon/Scene/Components.h"
#include "Neon/Scene/Entity.h"
#include "Scene.h"

namespace Neon
{
	std::unordered_map<UUID, Scene*> s_ActiveScenes;

	struct SceneComponent
	{
		UUID SceneID;
	};

	Scene::Scene(const std::string debugName /*= "Scene"*/)
		: m_DebugName(debugName)
	{
		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

		s_ActiveScenes[m_SceneID] = this;

		Init();
	}

	Scene::~Scene()
	{
		m_Registry.clear();
		s_ActiveScenes.erase(m_SceneID);
	}

	void Scene::Init()
	{
	}

	void Scene::OnUpdate(float deltaSeconds)
	{
	}

	void Scene::OnRenderEditor(float deltaSeconds, const EditorCamera& editorCamera)
	{
		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto& [meshComponent, transformComponent] = group.get<MeshComponent, TransformComponent>(entity);
			if (meshComponent.Mesh)
			{
				meshComponent.Mesh->OnUpdate(deltaSeconds);
			}
		}
	}

	void Scene::OnEvent(Event& e)
	{
	}

	void Scene::SetViewportSize(uint32 width, uint32 height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	Entity Scene::CreateEntity(const std::string& name /*= ""*/)
	{
		auto entity = Entity{m_Registry.create(), this};
		auto& idComponent = entity.AddComponent<IDComponent>();
		idComponent.ID = {};

		entity.AddComponent<TransformComponent>(glm::mat4(1.0f));
		if (!name.empty())
		{
			entity.AddComponent<TagComponent>(name);
		}

		m_EntityIDMap[idComponent.ID] = entity;
		return entity;
	}

	Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name /*= ""*/, bool runtimeMap /*= false*/)
	{
		auto entity = Entity{m_Registry.create(), this};
		auto& idComponent = entity.AddComponent<IDComponent>();
		idComponent.ID = uuid;

		entity.AddComponent<TransformComponent>(glm::mat4(1.0f));
		if (!name.empty())
		{
			entity.AddComponent<TagComponent>(name);
		}

		NEO_CORE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end());
		m_EntityIDMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity.m_EntityHandle);
	}

	Entity Scene::FindEntityByTag(const std::string& tag)
	{
		// TODO: Indexing by tag?
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const auto& canditate = view.get<TagComponent>(entity).Tag;
			if (canditate == tag)
			{
				return Entity(entity, this);
			}
		}

		return Entity{};
	}

	SharedRef<Scene> Scene::GetScene(UUID uuid)
	{
		if (s_ActiveScenes.find(uuid) != s_ActiveScenes.end())
		{
			return s_ActiveScenes.at(uuid);
		}

		return {};
	}

} // namespace Neon
