#pragma once

#include "Event/ApplicationEvent.h"
#include "Event/Event.h"
#include "Layer.h"
#include "Neon/Core/LayerStack.h"
#include "Neon/Core/Window.h"

#include <chrono>

namespace Neon
{
	class ImGuiLayer;

	struct ApplicationProps
	{
		std::string Name;
		uint32 WindowWidth;
		uint32 WindowHeight;

		ApplicationProps(const std::string& name = "Neon Engine", uint32 windowWidth = 1920, uint32 windowHeight = 1080)
			: Name(name)
			, WindowWidth(windowWidth)
			, WindowHeight(windowHeight)
		{
		}
	};

	class Application
	{
	public:
		explicit Application(const ApplicationProps& applicationProps = ApplicationProps());
		virtual ~Application();
		Application(const Application& other) = delete;
		Application& operator=(const Application& other) = delete;
		Application(const Application&& other) = delete;
		Application& operator=(const Application&& other) = delete;

		void Run();
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow()
		{
			return *m_Window;
		}

		static Application& Get() noexcept
		{
			return *s_Instance;
		}

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		UniqueRef<Window> m_Window;

		bool m_Running = true;
		bool m_Minimized = false;

		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		std::chrono::time_point<std::chrono::steady_clock> m_LastFrameTime = std::chrono::high_resolution_clock::now();

	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();

} // namespace Neon
