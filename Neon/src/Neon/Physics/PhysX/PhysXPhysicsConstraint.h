#pragma once

#include "Neon/Physics/PhysicsConstraint.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXPhysicsConstraint : public PhysicsConstraint
	{
	public:
		PhysXPhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1);
		virtual ~PhysXPhysicsConstraint() = default;

		virtual void* GetHandle() const override
		{
			NEO_CORE_ASSERT(m_Joint);
			return m_Joint;
		}

		virtual void Destroy() override;

	private:
		physx::PxD6Joint* m_Joint = nullptr;
	};
} // namespace Neon
