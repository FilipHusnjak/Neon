#include "neopch.h"

#include "InspectorPanel.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Renderer/Renderer.h"
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
				entity.RemoveComponent<T>();
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

	InspectorPanel::InspectorPanel()
	{
		m_CheckerboardTex = Texture2D::Create("assets/editor/Checkerboard.tga", {});
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
				if (DrawVec3Control("Rotation", originalEulerAngles))
				{
					// Wrap euler angles between -180 and 180 degrees
					originalEulerAngles -= 360.f * glm::floor((originalEulerAngles + glm::vec3(180.f)) / 360.f);
					component.Rotation = glm::radians(originalEulerAngles);
				}

				DrawVec3Control("Scale", component.Scale);
			});

			ImGui::Spacing();

			DrawComponent<MeshComponent>("Mesh Component", selectedEntity, [this](MeshComponent& component) {
				ImGui::BeginTable("##meshfiletable", 3);

				ImGui::TableSetupColumn("##meshfileTitle", 0, 100);
				ImGui::TableSetupColumn("##meshfile", 0, 700);
				ImGui::TableSetupColumn("##meshfileButton", 0, 40);

				ImGui::TableNextColumn();
				ImGui::Text("File Path");

				ImGui::TableNextColumn();
				if (component.Mesh)
				{
					ImGui::InputText("##meshfilepath", (char*)component.Mesh->GetFilePath().c_str(), 256,
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
						RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(component.Mesh));
						component.Mesh = SharedRef<Mesh>::Create(file);
					}
				}

				ImGui::EndTable();

				if (component.Mesh && ImGui::TreeNodeEx("Materials"))
				{
					ImGui::BeginTable("##meshmaterialssizetable", 2);

					ImGui::TableNextColumn();
					ImGui::Text("Size");

					ImGui::TableNextColumn();
					ImGui::InputText("##meshmaterialssize", (char*)std::to_string(component.Mesh->GetMaterials().size()).c_str(),
									 10);

					ImGui::EndTable();

					for (uint32 i = 0; i < component.Mesh->GetMaterials().size(); i++)
					{
						Material& material = component.Mesh->GetMaterials()[i];
						MaterialProperties materialProperties = material.GetProperties();

						std::string name = "Element " + std::to_string(i);
						if (ImGui::TreeNodeEx(name.c_str()))
						{
							if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								ImGui::Image(materialProperties.UseAlbedoMap > 0.5
												 ? material.GetTexture2D("u_AlbedoTextures")->GetRendererId()
												 : m_CheckerboardTex->GetRendererId(),
											 ImVec2(64, 64));
								ImGui::PopStyleVar();

								if (ImGui::IsItemClicked())
								{
									std::string filename = Application::Get().OpenFile("");
									if (!filename.empty())
									{
										SharedRef<Texture2D> albedoMap = Texture2D::Create(filename, {TextureType::SRGB});
										material.SetTexture2D("u_AlbedoTextures", albedoMap, 0);
									}
								}

								ImGui::SameLine();
								ImGui::BeginGroup();
								bool useAlbedoMap = materialProperties.UseAlbedoMap > 0.5f;
								if (ImGui::Checkbox("Use##AlbedoMap", &useAlbedoMap))
								{
									materialProperties.UseAlbedoMap = useAlbedoMap ? 1.f : 0.f;
								}
								ImGui::EndGroup();

								ImGui::SameLine();
								ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(materialProperties.AlbedoColor),
												  ImGuiColorEditFlags_NoInputs);
							}

							if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								ImGui::Image(materialProperties.UseNormalMap > 0.5
												 ? material.GetTexture2D("u_NormalTextures")->GetRendererId()
												 : m_CheckerboardTex->GetRendererId(),
											 ImVec2(64, 64));
								ImGui::PopStyleVar();

								if (ImGui::IsItemClicked())
								{
									std::string filename = Application::Get().OpenFile("");
									if (!filename.empty())
									{
										SharedRef<Texture2D> normalMap = Texture2D::Create(filename, {TextureType::RGB});
										material.SetTexture2D("u_NormalTextures", normalMap, 0);
										materialProperties.UseNormalMap = 1.f;
									}
								}

								ImGui::SameLine();
								ImGui::BeginGroup();
								bool useNormalMap = materialProperties.UseNormalMap > 0.5f;
								if (ImGui::Checkbox("Use##NormalMap", &useNormalMap))
								{
									materialProperties.UseNormalMap = useNormalMap ? 1.f : 0.f;
								}
								ImGui::EndGroup();
							}

							if (ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								ImGui::Image(materialProperties.UseMetalnessMap > 0.5
												 ? material.GetTexture2D("u_MetalnessTextures")->GetRendererId()
												 : m_CheckerboardTex->GetRendererId(),
											 ImVec2(64, 64));
								ImGui::PopStyleVar();

								if (ImGui::IsItemClicked())
								{
									std::string filename = Application::Get().OpenFile("");
									if (!filename.empty())
									{
										SharedRef<Texture2D> metalnessMap = Texture2D::Create(filename, {TextureType::SRGB});
										material.SetTexture2D("u_MetalnessTextures", metalnessMap, 0);
										materialProperties.UseMetalnessMap = 1.f;
									}
								}

								ImGui::SameLine();
								ImGui::BeginGroup();
								bool useMetalnessMap = materialProperties.UseMetalnessMap > 0.5f;
								if (ImGui::Checkbox("Use##MetalnessMap", &useMetalnessMap))
								{
									materialProperties.UseMetalnessMap = useMetalnessMap ? 1.f : 0.f;
								}
								ImGui::EndGroup();

								ImGui::SameLine();
								ImGui::SliderFloat("##MetalnessInput", &materialProperties.Metalness, 0.0f, 1.0f);
							}

							if (ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								ImGui::Image(materialProperties.UseRoughnessMap > 0.5
												 ? material.GetTexture2D("u_RoughnessTextures")->GetRendererId()
												 : m_CheckerboardTex->GetRendererId(),
											 ImVec2(64, 64));
								ImGui::PopStyleVar();

								if (ImGui::IsItemClicked())
								{
									std::string filename = Application::Get().OpenFile("");
									if (!filename.empty())
									{
										SharedRef<Texture2D> roughnessMap = Texture2D::Create(filename, {TextureType::SRGB});
										material.SetTexture2D("u_RoughnessTextures", roughnessMap, 0);
										materialProperties.UseRoughnessMap = 1.f;
									}
								}

								ImGui::SameLine();
								ImGui::BeginGroup();
								bool useRoughnessMap = materialProperties.UseRoughnessMap > 0.5f;
								if (ImGui::Checkbox("Use##RoughnessMap", &useRoughnessMap))
								{
									materialProperties.UseRoughnessMap = useRoughnessMap ? 1.f : 0.f;
								}
								ImGui::EndGroup();

								ImGui::SameLine();
								ImGui::SliderFloat("##RoughnessInput", &materialProperties.Roughness, 0.0f, 1.0f);
							}

							ImGui::TreePop();

							material.SetProperties(materialProperties);
						}

						ImGui::Spacing();
						ImGui::Spacing();
					}

					ImGui::TreePop();
				}
			});

			ImGui::Spacing();

			DrawComponent<LightComponent>("Light Component", selectedEntity, [](LightComponent& component) {

			});

			ImGui::Spacing();

			ImGui::Separator();

			float windowSizeX = ImGui::GetContentRegionAvail().x;
			ImGui::Indent((windowSizeX - 300.f) / 2.f);
			if (ImGui::Button("Add Component", ImVec2(300, 30)))
			{
				ImGui::OpenPopup("AddComponentPanel");
			}

			if (ImGui::BeginPopup("AddComponentPanel"))
			{
				if (!selectedEntity.HasComponent<MeshComponent>())
				{
					if (ImGui::Button("MeshComponent"))
					{
						selectedEntity.AddComponent<MeshComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				if (!selectedEntity.HasComponent<LightComponent>())
				{
					if (ImGui::Button("LightComponent"))
					{
						selectedEntity.AddComponent<LightComponent>();
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}
} // namespace Neon
