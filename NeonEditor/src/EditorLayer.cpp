#include "neopch.h"

#include "EditorLayer.h"

#include <Neon/Core/Event/KeyEvent.h>
#include <Neon/Editor/Panels/ContentBrowserPanel.h>
#include <Neon/Editor/Panels/InspectorPanel.h>
#include <Neon/Editor/Panels/SceneHierarchyPanel.h>
#include <Neon/Editor/Panels/SceneRendererPanel.h>
#include <Neon/Physics/Physics.h>
#include <Neon/Physics/PhysicsMaterial.h>
#include <Neon/Renderer/MeshFactory.h>
#include <Neon/Renderer/Renderer.h>
#include <Neon/Renderer/SceneRenderer.h>
#include <Neon/Scene/Actor.h>
#include <Neon/Scene/Actors/Car.h>
#include <Neon/Scene/Components/LightComponent.h>
#include <Neon/Scene/Components/OceanComponent.h>
#include <Neon/Scene/Components/SkeletalMeshComponent.h>
#include <Neon/Scene/Components/StaticMeshComponent.h>
#include <Neon/Scene/Components/CameraComponent.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#define AVG_FRAME_COUNT 1000

namespace Neon
{
	static SharedRef<CameraComponent> s_CameraComp;
	static Camera* s_ActiveCamera = nullptr;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
		m_EditorScene = SharedRef<Scene>::Create();
		m_EditorScene->Init();

		{
			auto& car = m_EditorScene->CreateActor<Car>(0, "Car");
			s_CameraComp = car->AddComponent<CameraComponent>(car.Ptr());
			s_CameraComp->SetPositionOffset(glm::vec3(0.f, 1.5f, -6.f));
			s_CameraComp->SetRotationOffset(glm::quat(glm::vec3(-0.1f, 3.14f, 0.f)));
			m_EditorScene->PossesPawn(car.Ptr());
		}

		SharedRef<PhysicsMaterial> matPlane = PhysicsMaterial::CreateMaterial(5.f, 3.f, 0.1f, 300.f);
		{
			auto& sphere = m_EditorScene->CreateStaticMeshActor<Actor>("assets/models/primitives/Sphere.fbx", 0, "Sphere",
																	   glm::vec3(1.f, 1.f, 1.f));
			sphere->SetTranslation(glm::vec3(-0.3f, 0.2f, 10.f));
			auto& sphereStaticMeshComp = sphere->GetRootComponent<StaticMeshComponent>();
			sphereStaticMeshComp->CreatePhysicsBody(PhysicsBodyType::Static, "", matPlane);
			sphereStaticMeshComp->GetPhysicsBody()->AddSpherePrimitive(1.f);
		}

		{
			auto& sphere = m_EditorScene->CreateStaticMeshActor<Actor>("assets/models/primitives/Sphere.fbx", 0, "Sphere",
																	   glm::vec3(1.f, 1.f, 1.f));
			sphere->SetTranslation(glm::vec3(-0.3f, 0.1f, 11.f));
			auto& sphereStaticMeshComp = sphere->GetRootComponent<StaticMeshComponent>();
			sphereStaticMeshComp->CreatePhysicsBody(PhysicsBodyType::Static, "", matPlane);
			sphereStaticMeshComp->GetPhysicsBody()->AddSpherePrimitive(1.f);
		}

		{
			auto& plane = m_EditorScene->CreateStaticMeshActor<Actor>("assets/models/primitives/Cube.fbx", 0, "Plane",
																	  glm::vec3(3000.f, 1.f, 3000.f));
			auto& planeStaticMeshComp = plane->GetRootComponent<StaticMeshComponent>();
			planeStaticMeshComp->CreatePhysicsBody(PhysicsBodyType::Static, "", matPlane);
			planeStaticMeshComp->GetPhysicsBody()->AddBoxPrimitive(glm::vec3(3000.f, 1.f, 3000.f));
		}

		{
			//auto& ocean = m_EditorScene->CreateEntity("Ocean");
			//ocean.AddComponent<OceanComponent>(512);
			//auto& oceanTransformComponent = ocean.GetComponent<TransformComponent>();
			//oceanTransformComponent.Translation = glm::vec3(-500.f, 0, -500.f);
		}

		{
			auto& light = m_EditorScene->CreateActor<Actor>(4, "DirectionalLight");
			light->AddComponent<LightComponent>(light.Ptr(), glm::normalize(glm::vec4{1.f, 0.3f, 1.f, 0.f}));
		}

		m_Panels.emplace_back(SharedRef<SceneHierarchyPanel>::Create());
		m_Panels.emplace_back(SharedRef<InspectorPanel>::Create());
		m_Panels.emplace_back(SharedRef<ContentBrowserPanel>::Create());
		m_Panels.emplace_back(SharedRef<SceneRendererPanel>::Create());
	}

	void EditorLayer::OnAttach()
	{
		m_PlayButtonTex = Texture2D::Create("assets/editor/PlayButton.png", {TextureUsageFlagBits::ShaderRead});
		m_StopButtonTex = Texture2D::Create("assets/editor/StopButton.png", {TextureUsageFlagBits::ShaderRead});

		s_ActiveCamera = &m_EditorCamera;
	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::Tick(float deltaSeconds)
	{
		if (m_SceneState == SceneState::Edit)
		{
			s_ActiveCamera = &m_EditorCamera;
		}
		else
		{
			s_ActiveCamera = s_CameraComp.Ptr();
		}

		m_Times.push(deltaSeconds * 1000.f);
		m_TimePassed += deltaSeconds * 1000.f;
		m_FrameCount++;
		if (m_FrameCount > AVG_FRAME_COUNT)
		{
			m_FrameCount = AVG_FRAME_COUNT;
			m_TimePassed -= m_Times.front();
			m_Times.pop();
		}

		Pawn* possesedPawn = m_EditorScene->GetPossesedPawn();
		possesedPawn->ProcessInput(m_CachedInput);
		m_EditorCamera.Tick(deltaSeconds);

		SceneRenderer::BeginScene(s_ActiveCamera);
		m_EditorScene->TickScene(deltaSeconds);
		SceneRenderer::EndScene();

		m_CachedInput.clear();
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

		auto& colors = ImGui::GetStyle().Colors;

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 500.f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		for (const auto& panel : m_Panels)
		{
			panel->Render();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.305f, 0.31f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.1505f, 0.151f, 0.5f));

		ImGui::Begin("##tool_bar", nullptr,
					 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			float size = ImGui::GetWindowHeight() - 4.f;
			ImGui::SameLine((ImGui::GetWindowContentRegionMax().x / 2.f) -
							(1.5f * (ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.x)) - (size / 2.f));
			SharedRef<Texture2D> buttonTex = m_SceneState == SceneState::Play ? m_StopButtonTex : m_PlayButtonTex;
			if (ImGui::ImageButton(buttonTex->GetRendererId(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0,
								   ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
			{
				if (m_SceneState == SceneState::Edit)
				{
					m_SceneState = SceneState::Play;
				}
				else
				{
					m_SceneState = SceneState::Edit;
				}
			}
		}

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		auto viewportSize = ImGui::GetContentRegionAvail();

		s_ActiveCamera->SetViewportSize((uint32)viewportSize.x, (uint32)viewportSize.y);

		ImGui::Image(Renderer::GetFinalImageId(), viewportSize);

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) {
			m_CachedInput.push_back({e.GetKeyCode(), KeyEventType::Pressed});
			return true;
		});
	}
} // namespace Neon
