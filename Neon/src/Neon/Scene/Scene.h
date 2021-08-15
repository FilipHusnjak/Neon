#pragma once

#include "Neon/Core/UUID.h"
#include "Neon/Editor/EditorCamera.h"

#include <glm/glm.hpp>

namespace Neon
{
	class Actor;
	class Pawn;
	class Material;

	class Scene : public RefCounted
	{
	public:
		Scene(const std::string name = "SampleScene");
		~Scene();

		void Init();

		void TickScene(float deltaSeconds);

		void SetViewportSize(uint32 width, uint32 height);

		template<typename T, typename... Args>
		SharedRef<T> CreateActor(UUID uuid, const std::string& name, Args&&... args)
		{
			static_assert(std::is_base_of<Actor, T>::value, "Class is not an Actor!");
			SharedRef<T> actor = m_Actors.emplace_back(SharedRef<T>::Create(this, name, uuid, args...)).As<T>();
			actor->Initialize();
			return actor;
		}

		template<typename T, typename... Args>
		SharedRef<T> CreateStaticMeshActor(const std::string& path, UUID uuid, const std::string& name, const glm::vec3& scale,
										  Args&&... args)
		{
			auto actor = CreateActor<T>(uuid, name, args...);

			SharedRef<StaticMesh> staticMesh = SharedRef<StaticMesh>::Create(path, scale);
			actor->AddRootComponent<StaticMeshComponent>(actor.Ptr(), staticMesh);

			return actor;
		}

		template<typename T, typename... Args>
		SharedRef<T> CreateSkeletalMeshActor(const std::string& path, UUID uuid, const std::string& name, Args&&... args)
		{
			auto actor = CreateActor<T>(uuid, name, args...);

			SharedRef<SkeletalMesh> skeletalMesh = SharedRef<SkeletalMesh>::Create(path);
			actor->AddRootComponent<SkeletalMeshComponent>(actor.Ptr(), skeletalMesh);

			return actor;
		}

		void DestroyActor(SharedRef<Actor> actor);

		void PossesPawn(Pawn* pawn)
		{
			m_PossesedPawn = pawn;
		}
		Pawn* GetPossesedPawn() const
		{
			return m_PossesedPawn;
		}

		UUID GetUUID() const
		{
			return m_SceneID;
		}

		static SharedRef<Scene> GetScene(UUID uuid);

		const std::string& GetName() const
		{
			return m_Name;
		}

	private:
		UUID m_SceneID;

		std::string m_Name;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		std::string m_EnvironmentPath;

		float m_SkyboxLod = 1.0f;

		std::vector<SharedRef<Actor>> m_Actors;

		Pawn* m_PossesedPawn = nullptr;

		friend class Actor;
		friend class SceneRenderer;
		friend class SceneHierarchyPanel;
	};
} // namespace Neon
