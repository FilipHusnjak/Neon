#pragma once

#include <Neon/Core/Event/Event.h>
#include <Neon/Core/Layer.h>
#include <Neon/Scene/Scene.h>

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

		void OnUpdate(float deltaSeconds) override;
		void OnImGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		SharedRef<Scene> m_EditorScene;

		EditorCamera m_EditorCamera;

		std::queue<float> m_Times;
		float m_TimePassed = 0.0f;
		int m_FrameCount = 0;

		glm::vec4 clearColor{1.f, 1.f, 1.f, 1.f};
	};
} // namespace Neon
