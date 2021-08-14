#include "neopch.h"

#include "Neon/Core/Input.h"

namespace Neon
{
	glm::vec2 Input::s_LastMousePosition = glm::vec2();
	glm::vec2 Input::s_MouseDelta = glm::vec2();

	void Input::CalculateMouseDelta()
	{
		const glm::vec2 mouse{GetMouseX(), GetMouseY()};
		s_MouseDelta = (mouse - s_LastMousePosition) * 0.003f;
		s_LastMousePosition = mouse;
	}
}
