#include "neopch.h"

#include "Neon/Physics/PhysicsConstraint.h"
#include "Neon/Scene/Actors/Car.h"

#include <GLFW/glfw3.h>

namespace Neon
{
	Car::Car(Scene* scene, const std::string& tag, UUID id, Transform transform /*= Transform()*/)
		: Pawn(scene, tag, id, transform)
	{
		SetTranslation(glm::vec3(0.f, 40.f, 0.f));
		SetRotation(glm::quat(glm::vec3(0.f, 2.f, 0.f)));

		SharedRef<PhysicsMaterial> matBody = PhysicsMaterial::CreateMaterial(10.f, 8.f, 0.1f, 800.f);
		SharedRef<PhysicsMaterial> matTire = PhysicsMaterial::CreateMaterial(5.f, 3.f, 0.3f, 300.f);

		m_MeshComponent =
			AddRootComponent<SkeletalMeshComponent>(this, SharedRef<SkeletalMesh>::Create("assets/models/zero/carSK.fbx"));

		m_MeshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "", matBody);
		Transform bodyTransform;
		bodyTransform.Translation = glm::vec3(0.f, 0.7f, 0.07f);
		m_MeshComponent->GetPhysicsBody()->AddBoxPrimitive(glm::vec3(1.f, 0.5, 1.f), bodyTransform);

		m_MeshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "wheel_rl", matTire);
		m_MeshComponent->GetPhysicsBody("wheel_rl")->AddSpherePrimitive(0.37f);
		SetupWheel("wheel_rl");

		m_MeshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "wheel_rr", matTire);
		m_MeshComponent->GetPhysicsBody("wheel_rr")->AddSpherePrimitive(0.37f);
		SetupWheel("wheel_rr");

		m_MeshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "wheel_fl", matTire);
		m_MeshComponent->GetPhysicsBody("wheel_fl")->AddSpherePrimitive(0.37f);
		SetupWheel("wheel_fl");

		m_MeshComponent->CreatePhysicsBody(PhysicsBodyType::Dynamic, "wheel_fr", matTire);
		m_MeshComponent->GetPhysicsBody("wheel_fr")->AddSpherePrimitive(0.37f);
		SetupWheel("wheel_fr");
	}

	void Car::SetupInputComponent(SharedRef<InputComponent> m_InputComponent)
	{
		Pawn::SetupInputComponent(m_InputComponent);

		m_InputComponent->BindAction({GLFW_KEY_W, KeyEventType::Pressed}, [this]() { m_Force *= -1; });
	}

	void Car::Tick(float deltaSeconds)
	{
		Pawn::Tick(deltaSeconds);

		if (m_MeshComponent)
		{
			//m_MeshComponent->AddForceLocal(m_Force);
		}
	}

	void Car::SetupWheel(const std::string& boneName)
	{
		auto constraint = PhysicsConstraint::Create(m_MeshComponent->GetPhysicsBody(boneName), m_MeshComponent->GetPhysicsBody());
		constraint->SetMotion(MotionAxis::RotationY, Motion::Free);
		constraint->SetMotion(MotionAxis::TranslationZ, Motion::Limited);
		constraint->SetDrive(DriveAxis::TranslationZ, 300000.f, 100.f, 300000.f);
		constraint->SetLinearLimit(0.07f);
		Transform driveTransform;
		driveTransform.Translation = glm::vec3(0.f, 0.f, 0.f);
		constraint->SetDrivePosition(driveTransform);
		constraint->SetDriveVelocity(glm::vec3(), glm::vec3());
	}

} // namespace Neon
