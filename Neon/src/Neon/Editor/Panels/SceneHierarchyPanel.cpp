#include "neopch.h"

#include "Neon/Editor/Panels/SceneHierarchyPanel.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Components/LightComponent.h"
#include "Neon/Scene/Components/StaticMeshComponent.h"
#include "Neon/Scene/Components/SkeletalMeshComponent.h"
#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Scene.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Neon
{
	static bool DrawActorNode(const SharedRef<Actor>& actor)
	{
		const char* name = actor->GetTag().c_str();

		ImGuiTreeNodeFlags flags =
			(actor == SceneRenderer::GetSelectedActor() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint32)actor->GetHandle(), flags, name);
		if (ImGui::IsItemClicked())
		{
			SceneRenderer::SetSelectedActor(actor);
		}

		bool actorDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
			{
				actorDeleted = true;
			}

			ImGui::EndPopup();
		}
		if (opened)
		{
			// TODO: Children
			ImGui::TreePop();
		}

		return actorDeleted;
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
		if (ImGui::TreeNodeEx((void*)(uint32)activeScene->m_SceneID, flags, activeScene->GetName().c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				SceneRenderer::SetSelectedActor({});
			}

			SharedRef<Actor> actorToDelete = nullptr;
			for (const auto& [k, v] : activeScene->m_ActorMap)
			{
				if (DrawActorNode(v) && SceneRenderer::GetSelectedActor() == v)
				{
					actorToDelete = v;
				}
			}
			if (actorToDelete)
			{
				SceneRenderer::DestroyActor(actorToDelete);
				SceneRenderer::SetSelectedActor({});
			}

			ImGui::TreePop();
		}

		if (ImGui::BeginPopupContextWindow(nullptr, 1, false))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Actor"))
				{
					auto newActor = SceneRenderer::CreateActor(0, "Actor");
					SceneRenderer::SetSelectedActor(newActor);
				}
				if (ImGui::MenuItem("Static Mesh Actor"))
				{
					auto newActor = SceneRenderer::CreateActor(0, "Static Mesh Actor");
					newActor->AddComponent<StaticMeshComponent>();
				}
				if (ImGui::MenuItem("Skeletal Mesh Actor"))
				{
					auto newActor = SceneRenderer::CreateActor(0, "Skeletal Mesh Actor");
					newActor->AddComponent<SkeletalMeshComponent>();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Directional Light Actor"))
				{
					auto newActor = SceneRenderer::CreateActor(0, "Directional Light Actor");
					newActor->AddComponent<LightComponent>();
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

} // namespace Neon
