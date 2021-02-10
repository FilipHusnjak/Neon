#include "neopch.h"

#include "Neon/Scene/Components.h"
#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>

namespace Neon
{
	void SceneHierarchyPanel::SetScene(const SharedRef<Scene>& scene)
	{
		m_Scene = scene;
	}

	void SceneHierarchyPanel::SetSelected(Entity entity)
	{
		m_SelectedEntity = entity;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (!m_Scene)
		{
			return;
		}

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		if (ImGui::TreeNodeEx((void*)(uint32)m_Scene->m_SceneEntity, flags, m_Scene->GetName().c_str()))
		{
			uint32 entityCount = 0, meshCount = 0;
			m_Scene->m_Registry.each([&](auto entity) {
				Entity e(entity, m_Scene.Ptr());
				if (e.HasComponent<IDComponent>())
				{
					DrawEntityNode(e);
				}
			});

			ImGui::TreePop();
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		const char* name = "Unnamed Entity";
		if (entity.HasComponent<TagComponent>())
		{
			name = entity.GetComponent<TagComponent>().Tag.c_str();
		}

		ImGuiTreeNodeFlags flags = (entity == m_SelectedEntity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint32)entity.GetHandle(), flags, name);
		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
			{
				entityDeleted = true;
			}

			ImGui::EndPopup();
		}
		if (opened)
		{
			// TODO: Children
			ImGui::TreePop();
		}

		// Defer deletion until end of node UI
		if (entityDeleted)
		{
			
		}
	}

} // namespace Neon
