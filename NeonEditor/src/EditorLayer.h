#pragma once

#include <Neon/Core/Event/Event.h>
#include <Neon/Core/Input.h>
#include <Neon/Core/Layer.h>
#include <Neon/Editor/Panels/Panel.h>
#include <Neon/Scene/Scene.h>
#include <Neon/Renderer/Texture.h>

#include <glm/glm.hpp>

#include <queue>

namespace Neon
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();

		void OnAttach() override;
		void OnDetach() override;

		void Tick(float deltaSeconds) override;
		void OnRenderGui() override;
		void OnEvent(Event& e) override;

	private:
		SharedRef<Scene> m_EditorScene;

		EditorCamera m_EditorCamera;

		enum class SceneState
		{
			Edit = 0,
			Play = 1
		};
		SceneState m_SceneState = SceneState::Edit;

		std::vector<SharedRef<Panel>> m_Panels;

		std::queue<float> m_Times;
		float m_TimePassed = 0.0f;
		int m_FrameCount = 0;

		glm::vec4 clearColor{1.f, 1.f, 1.f, 1.f};

		std::vector<KeyBinding> m_CachedInput;

		SharedRef<Texture2D> m_PlayButtonTex, m_StopButtonTex, m_PauseButtonTex, m_SimulateButtonTex;
	};
} // namespace Neon
