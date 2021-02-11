#include "neopch.h"

#include "Neon/Scene/Components.h"
#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Neon
{
	template<typename T>
	static void DrawComponent(const std::string& name, Entity entity)
	{
		if (entity.HasComponent<T>())
		{
			const auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();

			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
													 ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
													 ImGuiTreeNodeFlags_FramePadding;
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
				{
					removeComponent = true;
				}

				ImGui::EndPopup();
			}

			if (open)
			{
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				//entity.RemoveComponent<T>();
			}
		}
	}

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
		ImGui::Begin("Hierarchy");

		if (!m_Scene)
		{
			return;
		}

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		if (ImGui::TreeNodeEx((void*)(uint32)m_Scene->m_SceneEntity, flags, m_Scene->GetName().c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				m_SelectedEntity = {};
			}

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

		ImGui::Begin("Inspector");

		if (m_SelectedEntity)
		{
			const char* name = "Unnamed Entity";
			if (m_SelectedEntity.HasComponent<TagComponent>())
			{
				name = m_SelectedEntity.GetComponent<TagComponent>().Tag.c_str();
			}

			DrawComponent<TransformComponent>("Transform Component", m_SelectedEntity);
			DrawComponent<MeshComponent>("Mesh Component", m_SelectedEntity);
			DrawComponent<LightComponent>("Light Component", m_SelectedEntity);
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
