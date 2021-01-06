#include "neopch.h"

#include "Window.h"

namespace Neon
{
	Window::Window(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
	}
} // namespace Neon
