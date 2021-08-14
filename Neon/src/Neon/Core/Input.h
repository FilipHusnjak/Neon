#pragma once

#include "Neon/Core/Event/Event.h"

#include <vector>
#include <glm/glm.hpp>

namespace Neon
{
	enum class KeyEventType
	{
		Pressed,
		Released,
		Repeat,
		Typed
	};

	struct KeyBinding
	{
		int32 KeyCode;
		KeyEventType EventType;

		bool operator==(const KeyBinding& other) const
		{
			return KeyCode == other.KeyCode && EventType == other.EventType;
		}
	};

	class Input
	{
	public:
		static void CalculateMouseDelta();

		static bool IsKeyPressed(int key);

		static bool IsMouseButtonPressed(int button);

		static glm::vec2 GetMousePosition();

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
