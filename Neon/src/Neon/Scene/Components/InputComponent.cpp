#include "neopch.h"

#include "InputComponent.h"

namespace Neon
{
	InputComponent::InputComponent(Actor* owner)
		: ActorComponent(owner)
	{
	}

	void InputComponent::ProcessInput(const std::vector<KeyBinding>& input)
	{
		for (const auto& keyBinding : input)
		{
			if (m_ActionDelegateMap.find(keyBinding) != m_ActionDelegateMap.end())
			{
				if (m_ActionDelegateMap[keyBinding] != nullptr)
				{
					m_ActionDelegateMap[keyBinding]();
				}
			}
		}
	}

	void InputComponent::BindAction(KeyBinding keyBinding, ActionDelegate actionDelegate)
	{
		m_ActionDelegateMap[keyBinding] = actionDelegate;
	}

} // namespace Neon
