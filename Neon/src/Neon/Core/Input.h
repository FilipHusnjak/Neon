#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Neon
{
	class Input
	{
	public:
		static void CalculateMouseDelta();

		static bool IsKeyPressed(int key);

		static bool IsMouseButtonPressed(int button);

		static std::pair<float, float> GetMousePosition();

		static float GetMouseX();

		static float GetMouseY();

		static void EnableCursor();

		static void DisableCursor();

		static glm::vec2 GetMouseDelta()
		{
			return s_MouseDelta;
		}

	private:
		static glm::vec2 s_LastMousePosition;
		static glm::vec2 s_MouseDelta;
	};
} // namespace Neon
