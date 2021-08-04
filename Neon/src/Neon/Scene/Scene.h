#pragma once

#include "Neon/Core/UUID.h"
#include "Neon/Editor/EditorCamera.h"

#include <glm/glm.hpp>

namespace Neon
{
	class Actor;
	class Material;

	class Scene : public RefCounted
	{
	public:
		Scene(const std::string name = "SampleScene");
		~Scene();

		void Init();

		void TickScene(float deltaSeconds);
		void OnEvent(Event& e);

		void SetViewportSize(uint32 width, uint32 height);

		SharedRef<Actor> CreateActor(UUID uuid = 0, const std::string& name = "");
		SharedRef<Actor> CreateStaticMesh(const std::string& path, UUID uuid = 0, const std::string& name = "");
		SharedRef<Actor> CreateSkeletalMesh(const std::string& path, UUID uuid = 0, const std::string& name = "");
		void DestroyActor(SharedRef<Actor> actor);

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

		friend class Actor;
		friend class SceneRenderer;
		friend class SceneHierarchyPanel;
	};
} // namespace Neon
