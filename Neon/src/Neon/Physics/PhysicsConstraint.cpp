#include "neopch.h"

#include "PhysicsConstraint.h"

// TODO: Remove
#include "Neon/Physics/PhysX/PhysXPhysicsConstraint.h"

namespace Neon
{
	SharedRef<PhysicsConstraint> PhysicsConstraint::Create(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1)
	{
		return SharedRef<PhysXPhysicsConstraint>::Create(body0, body1);
	}

	PhysicsConstraint::PhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1)
		: m_Body0(body0)
		, m_Body1(body1)
	{
	}
} // namespace Neon
