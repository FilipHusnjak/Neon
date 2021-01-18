#include "neopch.h"

#include "Application.h"
#include "Neon/ImGui/ImGuiLayer.h"
#include "Neon/Renderer/Framebuffer.h"
#include "Neon/Renderer/Renderer.h"
#include "Neon/Renderer/SceneRenderer.h"

#include <imgui/imgui.h>

namespace Neon
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProps& applicationProps)
	{
		NEO_ASSERT(s_Instance == nullptr, "Application already exists");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(
			Window::Create(WindowProps{applicationProps.Name, applicationProps.WindowWidth, applicationProps.WindowHeight}));
		m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
		m_Window->SetVSync(false);

		m_ImGuiLayer = ImGuiLayer::Create();
		PushOverlay(m_ImGuiLayer);

		Renderer::Init();
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(e);
			if (e.Handled)
			{
				break;
			}
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::Run()
	{
		while (m_Running)
		{
			auto time = std::chrono::high_resolution_clock::now();
			auto timeStep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			float deltaSeconds = std::chrono::duration<float, std::chrono::seconds::period>(timeStep).count();

			m_Window->ProcessEvents();

			if (!m_Minimized)
			{
				m_Window->GetRenderContext()->BeginFrame();

				Renderer::Begin();

				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(deltaSeconds);
				}

				m_ImGuiLayer->Begin();

				RendererAPI::RenderAPICapabilities& caps = RendererAPI::GetCapabilities();
				ImGui::Begin("Renderer");
				ImGui::Text("Vendor: %s", caps.Vendor.c_str());
				ImGui::Text("Renderer: %s", caps.Renderer.c_str());
				ImGui::Text("Version: %s", caps.Version.c_str());
				ImGui::Text("Frame Time: %.2fms", deltaSeconds * 1000.0);
				ImGui::End();

				SceneRenderer::OnImGuiRender();

				for (Layer* layer : m_LayerStack)
				{
					layer->OnImGuiRender();
				}

				m_ImGuiLayer->End();

				Renderer::End();

				m_Window->SwapBuffers();
			}
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Window->GetRenderContext()->OnResize(e.GetWidth(), e.GetHeight());

		auto& fbs = FramebufferPool::Get().GetAll();
		for (auto& fb : fbs)
		{
			fb->Resize(e.GetWidth(), e.GetHeight());
		}

		m_Minimized = false;
		return false;
	}
} // namespace Neon
