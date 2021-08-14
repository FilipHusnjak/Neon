#pragma once

#include "Neon/Core/Input.h"
#include "Neon/Scene/Components/ActorComponent.h"

namespace std
{
	template<>
	struct hash<Neon::KeyBinding>
	{
		size_t operator()(const Neon::KeyBinding& k) const
		{
			size_t h1 = hash<size_t>()(k.KeyCode);
			size_t h2 = hash<size_t>()(static_cast<size_t>(k.EventType));
			return h1 ^ (h2 << 1);
		}
	};
} // namespace std

namespace Neon
{
	using ActionDelegate = std::function<void(void)>;
	using AxisDelegate = std::function<void(float)>;

	class InputComponent : public ActorComponent
	{
	public:
		InputComponent(Actor* owner);
		virtual ~InputComponent() = default;

		void ProcessInput(const std::vector<KeyBinding>& input);

		void BindAction(KeyBinding keyBinding, ActionDelegate actionDelegate);

	private:
		std::unordered_map<KeyBinding, ActionDelegate> m_ActionDelegateMap;
		std::unordered_map<int32, AxisDelegate> m_AxisDelegateMap;
	};
} // namespace Neon
