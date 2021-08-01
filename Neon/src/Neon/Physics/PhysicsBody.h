#pragma once

#include "Neon/Physics/PhysicsMaterial.h"
#include "Neon/Physics/PhysicsPrimitives.h"

namespace Neon
{
	class PhysicsActor;

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
		PhysicsBody();
		virtual ~PhysicsBody() = default;

		virtual void Destroy() = 0;

		virtual void* GetHandle() const = 0;

		void SetMaterial(const SharedRef<PhysicsMaterial>& material);

		virtual void AddSpherePrimitive(float radius = 1.f) = 0;

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

		PhysicsType m_PhysicsType = PhysicsType::Simulated;

	};
} // namespace Neon
