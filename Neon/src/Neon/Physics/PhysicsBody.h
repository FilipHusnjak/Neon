#pragma once

#include "Neon/Physics/PhysicsMaterial.h"
#include "Neon/Physics/PhysicsPrimitives.h"

namespace Neon
{
	class PhysicsActor;

	class PhysicsBody : public RefCounted
	{
	public:
		PhysicsBody(const PhysicsActor& physicsActor);
		virtual ~PhysicsBody() = default;

		virtual void Release() = 0;

		void SetMaterial(const SharedRef<PhysicsMaterial>& material);

		virtual void AddSpherePrimitive(float radius = 1.f) = 0;

		const PhysicsActor& GetPhysicsActor() const
		{
			return m_PhysicsActor;
		}

		const SharedRef<PhysicsMaterial>& GetMaterial() const
		{
			return m_Material;
		}

		bool IsValid() const
		{
			return m_Material.Ptr() != nullptr;
		}

	protected:
		const PhysicsActor& m_PhysicsActor;
		std::vector<UniqueRef<SpherePhysicsPrimitive>> m_Spheres;
		SharedRef<PhysicsMaterial> m_Material;
	};
} // namespace Neon
