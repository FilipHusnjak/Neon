#pragma once

namespace Neon
{
	class Actor;

	class ActorComponent
	{
	public:
		ActorComponent() = default;
		ActorComponent(Actor* owner);
		virtual ~ActorComponent() = default;

		virtual void TickComponent(float deltaSeconds);

		Actor* GetOwner() const
		{
			NEO_CORE_ASSERT(m_Owner);
			return m_Owner;
		}

	private:
		Actor* m_Owner = nullptr;
	};
} // namespace Neon
