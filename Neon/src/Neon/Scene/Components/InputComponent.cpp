#include "neopch.h"

#include "InputComponent.h"

namespace Neon
{
	InputComponent::InputComponent(Actor* owner)
		: ActorComponent(owner)
	{
	}

	void InputComponent::ProcessInput()
	{
		for (const auto& keyBinding : m_CachedInputs)
		{
		/*	if (m_ActionDelegateMap.find(keyBinding) != m_ActionDelegateMap.end())
			{
				if (m_ActionDelegateMap[keyBinding] != nullptr)
				{
					m_ActionDelegateMap[keyBinding]();
				}
			}*/
		}

		m_CachedInputs.clear();
	}

	void InputComponent::Input(KeyBinding keyBinding)
	{
		m_CachedInputs.push_back(keyBinding);
	}

} // namespace Neon
