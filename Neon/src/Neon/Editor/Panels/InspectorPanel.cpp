#include "neopch.h"

#include "Neon/Editor/Panels/InspectorPanel.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Renderer/Renderer.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Components/LightComponent.h"
#include "Neon/Scene/Components/OceanComponent.h"
#include "Neon/Scene/Components/SkeletalMeshComponent.h"
#include "Neon/Scene/Components/StaticMeshComponent.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
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
		SharedRef<Actor> selectedActor = SceneRenderer::GetSelectedActor();

		ImGui::Begin("Inspector");

		if (selectedActor)
		{
			const char* name = selectedActor->GetTag().c_str();

			DrawVec3Control("Translation", selectedActor->GetTranslation());

			glm::vec3 originalEulerAngles = glm::degrees(selectedActor->GetRotation());
			if (DrawVec3Control("Rotation", originalEulerAngles))
			{
				// Wrap euler angles between -180 and 180 degrees
				originalEulerAngles -= 360.f * glm::floor((originalEulerAngles + glm::vec3(180.f)) / 360.f);
				selectedActor->GetRotation() = glm::radians(originalEulerAngles);
			}

			DrawVec3Control("Scale", selectedActor->GetScale());

			ImGui::Spacing();

			DrawActorComponents(selectedActor);

			/*DrawComponent<StaticMeshComponent>("Static Mesh Component", selectedActor, [this](StaticMeshComponent& component) {
				ImGui::BeginTable("##meshfiletable", 3);

				ImGui::TableSetupColumn("##meshfileTitle", 0, 100);
				ImGui::TableSetupColumn("##meshfile", 0, 700);
				ImGui::TableSetupColumn("##meshfileButton", 0, 40);

				ImGui::TableNextColumn();
				ImGui::Text("File Path");

				ImGui::TableNextColumn();
				if (component.GetMesh())
				{
					ImGui::InputText("##meshfilepath", (char*)component.GetMesh()->GetFilePath().c_str(), 256,
									 ImGuiInputTextFlags_ReadOnly);
				}
				else
				{
					ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
				}

				ImGui::TableNextColumn();
				if (ImGui::Button("...##openmesh"))
				{
					std::string file = Application::Get().OpenFile();
					if (!file.empty())
					{
						RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(component.GetMesh()));
						component.LoadMesh(file);
					}
				}

				ImGui::EndTable();

				if (component.GetMesh() && ImGui::TreeNodeEx("Materials"))
				{
					RenderMeshProperties(component.GetMesh());
				}
			});

			ImGui::Spacing();

			DrawComponent<SkeletalMeshComponent>(
				"Skeletal Mesh Component", selectedActor, [this](SkeletalMeshComponent& component) {
					ImGui::BeginTable("##meshfiletable", 3);

					ImGui::TableSetupColumn("##meshfileTitle", 0, 100);
					ImGui::TableSetupColumn("##meshfile", 0, 700);
					ImGui::TableSetupColumn("##meshfileButton", 0, 40);

					ImGui::TableNextColumn();
					ImGui::Text("File Path");

					ImGui::TableNextColumn();
					if (component.GetMesh())
					{
						ImGui::InputText("##meshfilepath", (char*)component.GetMesh()->GetFilePath().c_str(), 256,
										 ImGuiInputTextFlags_ReadOnly);
					}
					else
					{
						ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
					}

					ImGui::TableNextColumn();
					if (ImGui::Button("...##openmesh"))
					{
						std::string file = Application::Get().OpenFile();
						if (!file.empty())
						{
							RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(component.GetMesh()));
							component.LoadMesh(file);
						}
					}

					ImGui::EndTable();

					if (component.GetMesh() && ImGui::TreeNodeEx("Materials"))
					{
						RenderMeshProperties(component.GetMesh());
					}
				});

			ImGui::Spacing();

			DrawComponent<LightComponent>("Light Component", selectedActor, [](LightComponent& component) {

			});

			DrawComponent<OceanComponent>("Ocean Component", selectedActor, [](OceanComponent& component) {

			});

			ImGui::Spacing();*/

			ImGui::Separator();

			float windowSizeX = ImGui::GetContentRegionAvail().x;
			ImGui::Indent((windowSizeX - 300.f) / 2.f);
			if (ImGui::Button("Add Component", ImVec2(300, 30)))
			{
				ImGui::OpenPopup("AddComponentPanel");
			}

			if (ImGui::BeginPopup("AddComponentPanel"))
			{
				if (ImGui::Button("StaticMeshComponent"))
				{
					selectedActor->AddComponent<StaticMeshComponent>(selectedActor.Ptr());
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Button("SkeletalMeshComponent"))
				{
					selectedActor->AddComponent<SkeletalMeshComponent>(selectedActor.Ptr());
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Button("LightComponent"))
				{
					selectedActor->AddComponent<LightComponent>(selectedActor.Ptr());
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

	void InspectorPanel::DrawActorComponents(SharedRef<Actor> actor) const
	{
		for (auto& component : actor->GetActorComponents())
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();

			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
													 ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
													 ImGuiTreeNodeFlags_FramePadding;
			bool open = ImGui::TreeNodeEx((void*)typeid(component).hash_code(), treeNodeFlags, typeid(component).name());
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
				component->RenderGui();
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				actor->RemoveComponent(component);
			}

			ImGui::Spacing();
		}
	}

} // namespace Neon
