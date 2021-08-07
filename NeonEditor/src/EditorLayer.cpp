#include "neopch.h"

#include "EditorLayer.h"

#include <Neon/Editor/Panels/ContentBrowserPanel.h>
#include <Neon/Editor/Panels/InspectorPanel.h>
#include <Neon/Editor/Panels/SceneHierarchyPanel.h>
#include <Neon/Editor/Panels/SceneRendererPanel.h>
#include <Neon/Physics/Physics.h>
#include <Neon/Renderer/MeshFactory.h>
#include <Neon/Renderer/Renderer.h>
#include <Neon/Renderer/SceneRenderer.h>
#include <Neon/Scene/Actor.h>
#include <Neon/Scene/Components/LightComponent.h>
#include <Neon/Scene/Components/OceanComponent.h>
#include <Neon/Scene/Components/StaticMeshComponent.h>

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

		SharedRef<PhysicsBody> body0;
		SharedRef<PhysicsBody> body1;
		{
			auto& cube = m_EditorScene->CreateStaticMesh("assets/models/primitives/Cube.fbx", 0, "Cube1", glm::vec3(1.f, 10.f, 1.f));
			cube->SetTranslation(glm::vec3(5.f, 45.f, 0.f));
			auto& cubeStaticMeshComp = cube->GetRootComponent<StaticMeshComponent>();
			cubeStaticMeshComp->CreatePhysicsBody(PhysicsBodyType::Dynamic);
			cubeStaticMeshComp->GetPhysicsBody()->AddBoxPrimitive(glm::vec3(1.f, 10.f, 1.f));
			body0 = cubeStaticMeshComp->GetPhysicsBody();
		}

		{
			auto& cube =
				m_EditorScene->CreateStaticMesh("assets/models/primitives/Cube.fbx", 0, "Cube2", glm::vec3(1.f, 10.f, 1.f));
			cube->SetTranslation(glm::vec3(0.f, 40.f, 0.f));
			auto& cubeStaticMeshComp = cube->GetRootComponent<StaticMeshComponent>();
			cubeStaticMeshComp->CreatePhysicsBody(PhysicsBodyType::Dynamic);
			cubeStaticMeshComp->GetPhysicsBody()->AddBoxPrimitive(glm::vec3(1.f, 10.f, 1.f));
			body1 = cubeStaticMeshComp->GetPhysicsBody();
		}

		Physics::GetCurrentScene()->AddPhysicsConstraint(body0, body1);
	
		{
			auto& sphere =
				m_EditorScene->CreateStaticMesh("assets/models/primitives/Sphere.fbx", 0, "Sphere", glm::vec3(1.f, 1.f, 1.f));
			sphere->SetTranslation(glm::vec3(0.5f, 10.f, 0.f));
			auto& sphereStaticMeshComp = sphere->GetRootComponent<StaticMeshComponent>();
			sphereStaticMeshComp->CreatePhysicsBody(PhysicsBodyType::Dynamic);
			sphereStaticMeshComp->GetPhysicsBody()->AddSpherePrimitive(1.f);
		}

		{
			auto& plane =
				m_EditorScene->CreateStaticMesh("assets/models/primitives/Cube.fbx", 0, "Plane", glm::vec3(30.f, 1.f, 30.f));
			auto& planeStaticMeshComp = plane->GetRootComponent<StaticMeshComponent>();
			planeStaticMeshComp->CreatePhysicsBody(PhysicsBodyType::Static);
			planeStaticMeshComp->GetPhysicsBody()->AddBoxPrimitive(glm::vec3(30.f, 1.f, 30.f));
		}

		{
			//auto& ocean = m_EditorScene->CreateEntity("Ocean");
			//ocean.AddComponent<OceanComponent>(512);
			//auto& oceanTransformComponent = ocean.GetComponent<TransformComponent>();
			//oceanTransformComponent.Translation = glm::vec3(-500.f, 0, -500.f);
		}

		{
			auto& light = m_EditorScene->CreateActor(4, "DirectionalLight");
			light->AddComponent<LightComponent>(light.Ptr(), glm::normalize(glm::vec4{1.f, 0.3f, 1.f, 0.f}));
		}

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

		Physics::TickPhysics(deltaSeconds);

		SceneRenderer::BeginScene({m_EditorCamera, 0.1f, 1000.0f, 45.0f});
		m_EditorScene->TickScene(deltaSeconds);
		SceneRenderer::EndScene();
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
