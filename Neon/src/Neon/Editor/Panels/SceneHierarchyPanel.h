#pragma once

#include "Neon/Scene/Entity.h"
#include "Neon/Scene/Scene.h"

namespace Neon
{
	class SceneHierarchyPanel
	{
	public:
		void SetScene(const SharedRef<Scene>& scene);
		void SetSelected(Entity entity);
		void OnImGuiRender();

	private:
		void DrawEntityNode(Entity entity);
		SharedRef<Scene> m_Scene;
		Entity m_SelectedEntity;
	};
} // namespace Neon
