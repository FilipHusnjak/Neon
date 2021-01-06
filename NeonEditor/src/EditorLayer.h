#pragma once

#include <Neon/Core/Layer.h>
#include <Neon/Core/Event/Event.h>

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

		void OnUpdate(float ts) override;
		void OnImGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		std::queue<float> m_Times;
		float m_TimePassed = 0.0f;
		int m_FrameCount = 0;

		glm::vec4 clearColor{1.f, 1.f, 1.f, 1.f};
	};
} // namespace Neon
