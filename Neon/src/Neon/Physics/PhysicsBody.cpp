#include "neopch.h"

#include "Neon/Physics/PhysicsBody.h"

namespace Neon
{
	PhysicsBody::PhysicsBody(PhysicsBodyType bodyType, const Transform& transform)
		: m_BodyType(bodyType)
	{
	}

	void PhysicsBody::SetMaterial(const SharedRef<PhysicsMaterial>& material)
	{
		m_Material = material;
	}

} // namespace Neon
