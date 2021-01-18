#pragma once

#include "Neon/Core/Window.h"

#include <GLFW/glfw3.h>

namespace Neon
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props = WindowProps());
		virtual ~WindowsWindow();

		void ProcessEvents() override;
		void SwapBuffers() override;

		inline unsigned int GetWidth() const override
		{
			return m_Data.Width;
		}
		inline unsigned int GetHeight() const override
		{
			return m_Data.Height;
		}

		std::pair<uint32_t, uint32_t> GetSize() const override
		{
			return {m_Data.Width, m_Data.Height};
		}
		std::pair<float, float> GetWindowPos() const override;

		void SetEventCallback(const EventCallbackFn& callback) override
		{
			m_Data.EventCallback = callback;
		}
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		const std::string& GetTitle() const override
		{
			return m_Data.Title;
		}
		void SetTitle(const std::string& title) override;

		inline void* GetNativeWindow() const override
		{
			return m_Window;
		}

		SharedRef<RendererContext> GetRenderContext() override
		{
			return m_RendererContext;
		}

	private:
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;
		GLFWcursor* m_ImGuiMouseCursors[9] = {};

		float m_LastFrameTime = 0.0f;

		SharedRef<RendererContext> m_RendererContext;
	};
} // namespace Neon
