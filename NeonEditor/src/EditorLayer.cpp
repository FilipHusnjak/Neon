#include "neopch.h"

#include "EditorLayer.h"

#include <Neon/Editor/Panels/ContentBrowserPanel.h>
#include <Neon/Editor/Panels/InspectorPanel.h>
#include <Neon/Editor/Panels/SceneHierarchyPanel.h>
#include <Neon/Editor/Panels/SceneRendererPanel.h>
#include <Neon/Renderer/Renderer.h>
#include <Neon/Scene/Components.h>
#include <Neon/Scene/Components/OceanComponent.h>
#include <Neon/Scene/Entity.h>

#include <imgui/imgui.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#define AVG_FRAME_COUNT 1000

namespace Neon
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		m_EditorScene = SharedRef<Scene>::Create();
		m_EditorScene->Init();

		auto& mesh = m_EditorScene->CreateMesh("assets/models/cube/cube.obj", "Entity1");
		auto& transformComponent = mesh.GetComponent<TransformComponent>();
		transformComponent.Rotation = {-PI / 2.f, 0.f, 0.f};

		auto& ocean = m_EditorScene->CreateEntity("Ocean");
		ocean.AddComponent<OceanComponent>(512);
		auto& oceanTransformComponent = ocean.GetComponent<TransformComponent>();
		//oceanTransformComponent.Translation = glm::vec3(-500.f, 0, -500.f);

		auto lightEntity = m_EditorScene->CreateEntity("DirectionalLight");
		lightEntity.AddComponent<LightComponent>(glm::normalize(glm::vec4{1.f, 0.3f, 1.f, 0.f}));

		m_Panels.emplace_back(SharedRef<SceneHierarchyPanel>::Create());
		m_Panels.emplace_back(SharedRef<InspectorPanel>::Create());
		m_Panels.emplace_back(SharedRef<ContentBrowserPanel>::Create());
		m_Panels.emplace_back(SharedRef<SceneRendererPanel>::Create());
	}

	void EditorLayer::OnAttach()
	{
	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnUpdate(float deltaSeconds)
	{
		m_Times.push(deltaSeconds * 1000.f);
		m_TimePassed += deltaSeconds * 1000.f;
		m_FrameCount++;
		if (m_FrameCount > AVG_FRAME_COUNT)
		{
			m_FrameCount = AVG_FRAME_COUNT;
			m_TimePassed -= m_Times.front();
			m_Times.pop();
		}

		m_EditorCamera.OnUpdate(deltaSeconds);

		m_EditorScene->OnUpdate(deltaSeconds);
		m_EditorScene->OnRenderEditor(deltaSeconds, m_EditorCamera);
	}

	void EditorLayer::OnRenderGui()
	{
		static bool dockSpaceOpen = true;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |=
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
		ImGui::PopStyleVar();

		ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 500.f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		for (const auto& panel : m_Panels)
		{
			panel->Render();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		auto viewportSize = ImGui::GetContentRegionAvail();

		m_EditorCamera.SetProjectionMatrix(
			glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 10000.0f));
		m_EditorCamera.SetViewportSize((uint32)viewportSize.x, (uint32)viewportSize.y);

		ImGui::Image(Renderer::GetFinalImageId(), viewportSize);

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);
	}
} // namespace Neon
