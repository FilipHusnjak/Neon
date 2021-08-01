#pragma once

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Scene/Components.h"
#include "Neon/Scene/Entity.h"

namespace Neon
{
	class PhysicsActor : public RefCounted
	{
	public:
		PhysicsActor(Entity entity);
		virtual ~PhysicsActor();

		float GetMass() const;
		void SetMass(float mass);

		virtual void* GetHandle() const = 0;

		Entity GetEntity() const
		{
			return m_Entity;
		}

	protected:
		Entity m_Entity;
		RigidBodyComponent& m_RigidBody;
		std::vector<PhysicsBody> m_PhysicsBodies;

	protected:
		friend class Physics;
	};
} // namespace Neon
