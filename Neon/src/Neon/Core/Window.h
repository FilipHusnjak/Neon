#pragma once

#include "Event/ApplicationEvent.h"
#include "Event/Event.h"
#include "Neon/Renderer/RendererContext.h"

#include <functional>

namespace Neon
{
	struct WindowProps
	{
		std::string Title;
		uint32 Width;
		uint32 Height;

		WindowProps(const std::string& title = "Neon Engine", uint32 width = 1920, uint32 height = 1080)
			: Title(title)
			, Width(width)
			, Height(height)
		{
		}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(const WindowProps& props);
		virtual ~Window() = default;

		virtual void ProcessEvents() = 0;
		virtual void SwapBuffers() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual std::pair<uint32_t, uint32_t> GetSize() const = 0;
		virtual std::pair<float, float> GetWindowPos() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual const std::string& GetTitle() const = 0;
		virtual void SetTitle(const std::string& title) = 0;

		virtual void* GetNativeWindow() const = 0;

		virtual SharedRef<RendererContext> GetRenderContext() = 0;

		static Window* Create(const WindowProps& props = WindowProps());

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

	protected:
		WindowData m_Data;
	};
} // namespace Neon
