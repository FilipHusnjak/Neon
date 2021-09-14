#pragma once

#include "Neon/Scene/Actors/Pawn.h"
#include "Neon/Scene/Components/SkeletalMeshComponent.h"

namespace Neon
{
	class Car : public Pawn
	{
	public:
		Car(Scene* scene, const std::string& tag, UUID id, Transform transform = Transform());

		virtual void SetupInputComponent(SharedRef<InputComponent> m_InputComponent) override;

		virtual void Tick(float deltaSeconds) override;

	private:
		void SetupWheel(const std::string& boneName);

	private:
		SharedRef<SkeletalMeshComponent> m_MeshComponent;

		glm::vec3 m_Force = glm::vec3(0.f, 0.f, 5000.f);
	};
} // namespace Neon
