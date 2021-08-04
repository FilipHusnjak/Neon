#pragma once

#include "Neon/Math/Math.h"
#include "Neon/Physics/PhysicsMaterial.h"
#include "Neon/Physics/PhysicsPrimitives.h"

namespace Neon
{
	class PhysicsActor;

	enum class PhysicsBodyType
	{
		Static,
		Dynamic
	};

	enum class PhysicsType
	{
		Simulated,
		Kinematic
	};

	enum class CollisionDetectionType
	{
		Discrete,
		Continuous,
		ContinuousSpeculative
	};

	class PhysicsBody : public RefCounted
	{
	public:
		PhysicsBody(PhysicsBodyType bodyType, const Transform& transform = Transform());
		virtual ~PhysicsBody() = default;

		virtual void Destroy() = 0;

		virtual void* GetHandle() const = 0;

		void SetMaterial(const SharedRef<PhysicsMaterial>& material);

		virtual void AddSpherePrimitive(float radius = 1.f) = 0;

		virtual Transform GetBodyTransform() const = 0;

		virtual glm::vec3 GetBodyTranslation() const = 0;
		virtual glm::vec3 GetBodyRotation() const = 0;

		const SharedRef<PhysicsMaterial>& GetMaterial() const
		{
			return m_Material;
		}

		bool IsValid() const
		{
			return GetHandle() != nullptr;
		}

	protected:
		std::vector<UniqueRef<SpherePhysicsPrimitive>> m_Spheres;
		SharedRef<PhysicsMaterial> m_Material;

		float m_Mass = 1.f;

		CollisionDetectionType CollisionDetection = CollisionDetectionType::Discrete;

		float m_LinearDamping = 0.01f;
		float m_AngularDamping = 0.0f;
		float m_EnableGravity = true;

		PhysicsBodyType m_BodyType;
		PhysicsType m_PhysicsType = PhysicsType::Simulated;
	};
} // namespace Neon
