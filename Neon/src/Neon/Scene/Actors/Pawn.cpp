#include "neopch.h"

#include "Pawn.h"

namespace Neon
{
	Pawn::Pawn(Scene* scene, const std::string& tag, UUID id, Transform transform /*= Transform()*/)
		: Actor(scene, tag, id, transform)
	{
		m_InputComponent = SharedRef<InputComponent>::Create(this);
	}

	void Pawn::Initialize()
	{
		Actor::Initialize();

		SetupInputComponent(m_InputComponent);
	}

	void Pawn::ProcessInput(const std::vector<KeyBinding>& input)
	{
		m_InputComponent->ProcessInput(input);
	}

} // namespace Neon
