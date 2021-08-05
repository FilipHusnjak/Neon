#pragma once

#include "Neon/Physics/PhysicsBody.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXPhysicsBody : public PhysicsBody
	{
	public:
		PhysXPhysicsBody(PhysicsBodyType bodyType, const Transform& transform);

		virtual void Destroy() override;

		virtual void AddSpherePrimitive(float radius = 1.f) override;
		virtual void AddBoxPrimitive(glm::vec3 size = glm::vec3(1.f)) override;
		virtual void AddCapsulePrimitive(float radius, float height) override;

		virtual Transform GetBodyTransform() const override;

		virtual glm::vec3 GetBodyTranslation() const override;
		virtual glm::vec3 GetBodyRotation() const override;

		virtual void* GetHandle() const override
		{
			return m_RigidActor;
		}

	private:
		physx::PxRigidActor* m_RigidActor = nullptr;
	};
} // namespace Neon
