#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsConstraint.h"
#include "Neon/Physics/Physics.h"
#include "Neon/Physics/PhysicsConstraint.h"

namespace Neon
{
	SharedRef<PhysicsConstraint> PhysicsConstraint::Create(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1)
	{
		switch (Physics::GetCurrentEngine())
		{
			case PhysicsEngine::PhysX:
				return SharedRef<PhysXPhysicsConstraint>::Create(body0, body1);
			default:
				NEO_CORE_ASSERT(false);
				return nullptr;
		}
	}

	PhysicsConstraint::PhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1)
		: m_Body0(body0)
		, m_Body1(body1)
	{
	}
} // namespace Neon
