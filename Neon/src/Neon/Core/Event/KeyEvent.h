#pragma once

#include "Event.h"

namespace Neon
{
class KeyEvent : public Event
{
public:
	[[nodiscard]] inline int GetKeyCode() const
	{
		return m_KeyCode;
	}

	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput);

protected:
	int m_KeyCode;

protected:
	explicit KeyEvent(int keycode)
		: m_KeyCode(keycode)
	{
	}
};
class KeyPressedEvent : public KeyEvent
{
public:
	explicit KeyPressedEvent(int keycode)
		: KeyEvent(keycode)
	{
	}

	EVENT_CLASS_TYPE(KeyPressed)
};
class KeyReleasedEvent : public KeyEvent
{
public:
	explicit KeyReleasedEvent(int keycode)
		: KeyEvent(keycode)
	{
	}

	EVENT_CLASS_TYPE(KeyReleased)
};
class KeyTypedEvent : public KeyEvent
{
public:
	explicit KeyTypedEvent(int keycode)
		: KeyEvent(keycode)
	{
	}

	EVENT_CLASS_TYPE(KeyTyped)
};
} // namespace Neon
