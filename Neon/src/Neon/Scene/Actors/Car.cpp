#include "neopch.h"

#include "Car.h"

#include <GLFW/glfw3.h>

namespace Neon
{
	Car::Car(Scene* scene, const std::string& tag, UUID id, Transform transform /*= Transform()*/)
		: Pawn(scene, tag, id, transform)
	{
		SetTranslation(glm::vec3(0.f, 40.f, 0.f));

		SharedRef<PhysicsMaterial> matBody = PhysicsMaterial::CreateMaterial(10.f, 8.f, 0.1f, 800.f);
		SharedRef<PhysicsMaterial> matTire = PhysicsMaterial::CreateMaterial(5.f, 3.f, 0.3f, 300.f);

		meshComponent =
			AddRootComponent<SkeletalMeshComponent>(this, SharedRef<SkeletalMesh>::Create("assets/models/zero/carSK.fbx"));

		meshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "", matBody);
		Transform bodyTransform;
		bodyTransform.Translation = glm::vec3(0.f, 0.7f, 0.07f);
		meshComponent->GetPhysicsBody()->AddBoxPrimitive(glm::vec3(1.f, 0.5, 1.f), bodyTransform);
		NEO_CORE_INFO("Mass body {0}", meshComponent->GetPhysicsBody()->GetMass());

		meshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "wheel_rl", matTire);
		meshComponent->GetPhysicsBody("wheel_rl")->AddSpherePrimitive(0.37f);
		NEO_CORE_INFO("Mass wheel {0}", meshComponent->GetPhysicsBody("wheel_rl")->GetMass());

		meshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "wheel_rr", matTire);
		meshComponent->GetPhysicsBody("wheel_rr")->AddSpherePrimitive(0.37f);

		meshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "wheel_fl", matTire);
		meshComponent->GetPhysicsBody("wheel_fl")->AddSpherePrimitive(0.37f);

		meshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "wheel_fr", matTire);
		meshComponent->GetPhysicsBody("wheel_fr")->AddSpherePrimitive(0.37f);
	}

	void Car::SetupInputComponent(SharedRef<InputComponent> m_InputComponent)
	{
		Pawn::SetupInputComponent(m_InputComponent);

		m_InputComponent->BindAction({GLFW_KEY_W, KeyEventType::Pressed}, [this]() { m_Force *= -1; });
	}

	void Car::Tick(float deltaSeconds)
	{
		Pawn::Tick(deltaSeconds);

		if (meshComponent)
		{
			meshComponent->AddForceLocal(m_Force);
		}
	}

} // namespace Neon
