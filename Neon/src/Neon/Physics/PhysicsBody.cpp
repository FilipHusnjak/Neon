#include "neopch.h"

#include "Neon/Physics/PhysicsBody.h"

namespace Neon
{
	PhysicsBody::PhysicsBody()
	{
	}

	void PhysicsBody::SetMaterial(const SharedRef<PhysicsMaterial>& material)
	{
		m_Material = material;
	}

} // namespace Neon
