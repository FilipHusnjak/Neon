#include "neopch.h"

#include "InputComponent.h"

namespace Neon
{

	InputComponent::InputComponent(Actor* owner)
		: ActorComponent(owner)
	{
	}

	void InputComponent::ProcessInput(const std::vector<KeyBinding>& keyBindings)
	{
	}

} // namespace Neon
