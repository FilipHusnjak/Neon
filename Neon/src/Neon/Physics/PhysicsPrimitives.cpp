#include "neopch.h"

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Physics/PhysicsPrimitives.h"
#include "Neon/Renderer/MeshFactory.h"

namespace Neon
{
	PhysicsPrimitive::PhysicsPrimitive(const PhysicsBody& physicsBody, const Transform& transform)
		: m_InternalShape(nullptr), m_Transform(transform)
	{
	}

	SpherePhysicsPrimitive::SpherePhysicsPrimitive(const PhysicsBody& physicsBody, float radius,
												   const Transform& transform)
		: PhysicsPrimitive(physicsBody, transform)
		, m_Radius(radius)
	{
		m_DebugMesh = MeshFactory::CreateSphere(radius);
	}

	BoxPhysicsPrimitive::BoxPhysicsPrimitive(const PhysicsBody& physicsBody, glm::vec3 size,
											 const Transform& transform)
		: PhysicsPrimitive(physicsBody, transform)
		, m_Size(size)
	{
		m_DebugMesh = MeshFactory::CreateBox(size);
	}

	CapsulePhysicsPrimitive::CapsulePhysicsPrimitive(const PhysicsBody& physicsBody, float radius, float height,
													 const Transform& transform)
		: PhysicsPrimitive(physicsBody, transform)
		, m_Radius(radius)
		, m_Height(height)
	{
		m_DebugMesh = MeshFactory::CreateCapsule(radius, height);
	}

} // namespace Neon
