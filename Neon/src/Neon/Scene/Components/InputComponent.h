#pragma once

#include "Neon/Scene/Components/ActorComponent.h"

namespace Neon
{
	using ActionDelegate = int ();
	using AxisDelegate = int (int);

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

		void ProcessInput();

		void Input(KeyBinding keyBinding);

	private:
		std::vector<KeyBinding> m_CachedInputs;
		//std::map<KeyBinding, ActionDelegate*> m_ActionDelegateMap;
		std::map<int32, AxisDelegate*> m_AxisDelegateMap;
	};
} // namespace Neon
