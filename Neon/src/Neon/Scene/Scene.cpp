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
		s_ActiveScenes.erase(m_SceneID);
	}

	void Scene::Init()
	{
		SceneRenderer::InitializeScene(this);
	}

	void Scene::TickScene(float deltaSeconds)
	{
		for (auto& actor : m_Actors)
		{
			NEO_CORE_ASSERT(actor);
			actor->Tick(deltaSeconds);
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

	SharedRef<Actor> Scene::CreateActor(UUID uuid, const std::string& name /*= ""*/)
	{
		return m_Actors.emplace_back(SharedRef<Actor>::Create(this, name, uuid));
	}

	SharedRef<Actor> Scene::CreateStaticMesh(const std::string& path, UUID uuid, const std::string& name /*= ""*/)
	{
		auto actor = CreateActor(uuid, name);

		SharedRef<StaticMesh> staticMesh = SharedRef<StaticMesh>::Create(path);
		actor->AddRootComponent<StaticMeshComponent>(actor.Ptr(), staticMesh);

		return actor;
	}

	SharedRef<Actor> Scene::CreateSkeletalMesh(const std::string& path, UUID uuid, const std::string& name /*= ""*/)
	{
		auto actor = CreateActor(uuid, name);

		SharedRef<SkeletalMesh> skeletalMesh = SharedRef<SkeletalMesh>::Create(path);
		actor->AddRootComponent<SkeletalMeshComponent>(actor.Ptr(), skeletalMesh);

		return actor;
	}

	void Scene::DestroyActor(SharedRef<Actor> actor)
	{
		for (auto it = m_Actors.begin(); it != m_Actors.end();)
		{
			if (it->Ptr() == actor.Ptr())
			{
				it = m_Actors.erase(it);
			}
			else
			{
				++it;
			}
		}
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
