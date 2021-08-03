#include "neopch.h"

#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Components/LightComponent.h"
#include "Neon/Scene/Components/OceanComponent.h"
#include "Neon/Scene/Components/SkeletalMeshComponent.h"
#include "Neon/Scene/Components/StaticMeshComponent.h"
#include "Neon/Scene/Scene.h"

namespace Neon
{
	static std::unordered_map<UUID, Scene*> s_ActiveScenes;

	struct SceneComponent
	{
		UUID SceneID;
	};

	Scene::Scene(const std::string name /*= "SampleScene"*/)
		: m_Name(name)
	{
		s_ActiveScenes[m_SceneID] = this;

		m_EnvironmentPath = "assets/textures/environment/daytime.hdr";
	}

	Scene::~Scene()
	{
		m_Registry.clear();
		s_ActiveScenes.erase(m_SceneID);
	}

	void Scene::Init()
	{
		SceneRenderer::InitializeScene(this);
	}

	void Scene::TickScene(float deltaSeconds)
	{
		for (auto& [k, actor] : m_ActorMap)
		{
			NEO_CORE_ASSERT(actor);
			actor->Tick(deltaSeconds);
		}
	}

	void Scene::OnRenderEditor(const EditorCamera& editorCamera)
	{
		SceneRenderer::BeginScene({editorCamera, 0.1f, 1000.0f, 45.0f});
		auto group0 = m_Registry.view<StaticMeshComponent>();
		for (auto entity : group0)
		{
			auto& meshComponent = group0.get<StaticMeshComponent>(entity);
			SharedRef<Actor> actor = GetActor(entity);
			if (meshComponent.GetMesh())
			{
				SceneRenderer::SubmitMesh(meshComponent.GetMesh(), actor->GetTransformMat());
			}
		}
		auto group1 = m_Registry.view<SkeletalMeshComponent>();
		for (auto entity : group1)
		{
			auto& meshComponent = group1.get<SkeletalMeshComponent>(entity);
			SharedRef<Actor> actor = GetActor(entity);
			if (meshComponent.GetMesh())
			{
				SceneRenderer::SubmitMesh(meshComponent.GetMesh(), actor->GetTransformMat());
			}
		}
		auto group2 = m_Registry.view<OceanComponent>();
		for (auto entity : group2)
		{
			auto& oceanComponent = group2.get<OceanComponent>(entity);
			SharedRef<Actor> actor = GetActor(entity);
			if (oceanComponent.operator SharedRef<Mesh>())
			{
				SceneRenderer::SubmitMesh(oceanComponent, actor->GetTransformMat());
			}
		}
		SceneRenderer::EndScene();
	}

	void Scene::OnEvent(Event& e)
	{
	}

	void Scene::SetViewportSize(uint32 width, uint32 height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	SharedRef<Actor> Scene::CreateActor(UUID uuid, const std::string& name /*= ""*/)
	{
		entt::entity entity = m_Registry.create();
		auto actor = SharedRef<Actor>::Create(entity, this, name, uuid);
		m_ActorMap[entity] = actor;
		return actor;
	}

	SharedRef<Actor> Scene::CreateStaticMesh(const std::string& path, UUID uuid, const std::string& name /*= ""*/)
	{
		auto actor = CreateActor(uuid, name);

		SharedRef<StaticMesh> staticMesh = SharedRef<StaticMesh>::Create(path);
		actor->AddComponent<StaticMeshComponent>(actor.Ptr(), staticMesh);

		return actor;
	}

	SharedRef<Actor> Scene::CreateSkeletalMesh(const std::string& path, UUID uuid, const std::string& name /*= ""*/)
	{
		auto actor = CreateActor(uuid, name);

		SharedRef<SkeletalMesh> skeletalMesh = SharedRef<SkeletalMesh>::Create(path);
		actor->AddComponent<SkeletalMeshComponent>(actor.Ptr(), skeletalMesh);

		return actor;
	}

	void Scene::DestroyActor(SharedRef<Actor> actor)
	{
		m_ActorMap.erase(actor->m_EntityHandle);
		m_Registry.destroy(actor->m_EntityHandle);
	}

	const SharedRef<Actor>& Scene::GetActor(entt::entity entity) const
	{
		NEO_CORE_ASSERT(m_ActorMap.find(entity) != m_ActorMap.end());
		NEO_CORE_ASSERT(m_ActorMap.at(entity));
		return m_ActorMap.at(entity);
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
