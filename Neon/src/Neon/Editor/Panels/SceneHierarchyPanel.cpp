#include "neopch.h"

#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Components.h"
#include "Neon/Scene/Entity.h"
#include "Neon/Scene/Scene.h"
#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Neon
{
	static void DrawEntityNode(Entity entity)
	{
		const char* name = "Unnamed Entity";
		if (entity.HasComponent<TagComponent>())
		{
			name = entity.GetComponent<TagComponent>().Tag.c_str();
		}

		ImGuiTreeNodeFlags flags =
			(entity == SceneRenderer::GetSelectedEntity() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint32)entity, flags, name);
		if (ImGui::IsItemClicked())
		{
			SceneRenderer::SetSelectedEntity(entity);
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
			if (SceneRenderer::GetSelectedEntity() == entity)
			{
				SceneRenderer::SetSelectedEntity({});
			}
			SceneRenderer::DestroyEntity(entity);
		}
	}

	void SceneHierarchyPanel::Render() const
	{
		SharedRef<Scene> activeScene = SceneRenderer::GetActiveScene();
		if (!activeScene)
		{
			return;
		}

		ImGui::Begin("Hierarchy");

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		if (ImGui::TreeNodeEx((void*)(uint32)activeScene->m_SceneEntity, flags, activeScene->GetName().c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				SceneRenderer::SetSelectedEntity({});
			}

			uint32 entityCount = 0, meshCount = 0;
			activeScene->m_Registry.each([&](auto entity) {
				Entity e(entity, activeScene.Ptr());
				if (e.HasComponent<IDComponent>())
				{
					DrawEntityNode(e);
				}
			});

			ImGui::TreePop();
		}

		if (ImGui::BeginPopupContextWindow(nullptr, 1, false))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Empty Entity"))
				{
					auto newEntity = SceneRenderer::CreateEntity("Empty Entity");
					SceneRenderer::SetSelectedEntity(newEntity);
				}
				if (ImGui::MenuItem("Mesh"))
				{
					auto newEntity = SceneRenderer::CreateEntity("Mesh Entity");
					newEntity.AddComponent<MeshComponent>();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Directional Light"))
				{
					auto newEntity = SceneRenderer::CreateEntity("Directional Light Entity");
					newEntity.AddComponent<LightComponent>();
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

} // namespace Neon
