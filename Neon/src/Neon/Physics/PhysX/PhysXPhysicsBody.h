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

		virtual void AddSpherePrimitive(float radius = 1.f, const Transform& transform = Transform()) override;
		virtual void AddBoxPrimitive(glm::vec3 size = glm::vec3(1.f), const Transform& transform = Transform()) override;
		virtual void AddCapsulePrimitive(float radius, float height, const Transform& transform = Transform()) override;

		virtual Transform GetBodyTransform() const override;

		virtual glm::vec3 GetBodyTranslation() const override;
		virtual glm::quat GetBodyRotation() const override;

		virtual void* GetHandle() const override
		{
			return m_RigidActor;
		}

	private:
		physx::PxRigidActor* m_RigidActor = nullptr;
	};
} // namespace Neon
