#include "neopch.h"

#include "InspectorPanel.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Components.h"
#include "Neon/Scene/Entity.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
	template<typename T, typename UIFunc>
	static void DrawComponent(const std::string& name, Entity entity, UIFunc uiFunc)
	{
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
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
			if (ImGui::Button("::", ImVec2{lineHeight, lineHeight}))
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
				uiFunc(component);
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				//entity.RemoveComponent<T>();
			}
		}
	}

	static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.f, float columnWidth = 120.f)
	{
		bool modified = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
			modified = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		modified |= ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return modified;
	}

	void InspectorPanel::Render() const
	{
		Entity selectedEntity = SceneRenderer::GetSelectedEntity();

		ImGui::Begin("Inspector");

		if (selectedEntity)
		{
			const char* name = "Unnamed Entity";
			if (selectedEntity.HasComponent<TagComponent>())
			{
				name = selectedEntity.GetComponent<TagComponent>().Tag.c_str();
			}

			DrawComponent<TransformComponent>("Transform Component", selectedEntity, [](TransformComponent& component) {
				DrawVec3Control("Translation", component.Translation);

				glm::vec3 originalEulerAngles = glm::degrees(component.Rotation);
				originalEulerAngles.x *= 2.f;
				if (DrawVec3Control("Rotation", originalEulerAngles))
				{
					if (originalEulerAngles.x >= 180.f)
					{
						originalEulerAngles.x -= 360.f;
					}
					if (originalEulerAngles.y >= 180.f)
					{
						originalEulerAngles.y -= 360.f;
					}
					if (originalEulerAngles.z >= 180.f)
					{
						originalEulerAngles.z -= 360.f;
					}

					if (originalEulerAngles.x <= -180.f)
					{
						originalEulerAngles.x += 360.f;
					}
					if (originalEulerAngles.y <= -180.f)
					{
						originalEulerAngles.y += 360.f;
					}
					if (originalEulerAngles.z <= -180.f)
					{
						originalEulerAngles.z += 360.f;
					}

					originalEulerAngles.x /= 2.f;
					component.Rotation = glm::radians(originalEulerAngles);
				}

				DrawVec3Control("Scale", component.Scale);
			});
			DrawComponent<MeshComponent>("Mesh Component", selectedEntity, [](MeshComponent& component) {
				ImGui::Columns(3);
				ImGui::SetColumnWidth(0, 100);
				ImGui::SetColumnWidth(1, 300);
				ImGui::SetColumnWidth(2, 40);
				ImGui::Text("File Path");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				if (component.Mesh)
				{
					ImGui::InputText("##meshfilepath", (char*)component.Mesh->GetFilePath().c_str(), 256,
									 ImGuiInputTextFlags_ReadOnly);
				}
				else
				{
					ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
				}
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::Columns(1);
			});
			DrawComponent<LightComponent>("Light Component", selectedEntity, [](LightComponent& component) {

			});
		}

		ImGui::End();
	}
} // namespace Neon
