#pragma once

#include "Neon/Scene/Components/ActorComponent.h"

namespace Neon
{
	typedef int (*ActionDelegate)();
	typedef int (*AxisDelegate)(int);

	enum class KeyEventType
	{
		Pressed,
		Released,
		Repeat
	};

	struct KeyBinding
	{
		int32 KeyCode;
		KeyEventType EventType;
	};

	class InputComponent : public ActorComponent
	{
	public:
		InputComponent(Actor* owner);
		virtual ~InputComponent() = default;

		void ProcessInput(const std::vector<KeyBinding>& keyBindings);

		void TickComponent();

	private:
		std::vector<KeyBinding> m_CachedInputs;
		std::map<KeyBinding, ActionDelegate> m_KeyDelegateMap;
		std::map<int32, AxisDelegate> m_AxisDelegateMap;
	};
} // namespace Neon
