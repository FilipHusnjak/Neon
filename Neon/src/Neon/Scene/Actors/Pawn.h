#pragma once

#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Components/InputComponent.h"

namespace Neon
{
	class Pawn : public Actor
	{
	public:
		Pawn(Scene* scene, const std::string& tag, UUID id, Transform transform = Transform());
		virtual ~Pawn() = default;

		virtual void Initialize() override;

		void ProcessInput(const std::vector<KeyBinding>& input);

		virtual void SetupInputComponent(SharedRef<InputComponent> m_InputComponent)
		{
		}

	private:
		SharedRef<InputComponent> m_InputComponent;
	};

} // namespace Neon
