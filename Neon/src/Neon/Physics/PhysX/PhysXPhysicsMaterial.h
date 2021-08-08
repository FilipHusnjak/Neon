#pragma once

#include "Neon/Physics/PhysicsMaterial.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Neon
{
	class PhysXPhysicsMaterial : public PhysicsMaterial
	{
	public:
		PhysXPhysicsMaterial(float staticFriction, float dynamicFriction, float restitution, float density);

		virtual void* GetHandle() const override
		{
			return m_Handle;
		}

	private:
		physx::PxMaterial* m_Handle;
	};
} // namespace Neon
