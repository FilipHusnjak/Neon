#pragma once

#include "Neon/Renderer/StaticMesh.h"

#include <glm/glm.hpp>

namespace Neon
{
	class PhysicsBody;

	class PhysicsPrimitive : public RefCounted
	{
	public:
		PhysicsPrimitive(const PhysicsBody& physicsBody);
		virtual ~PhysicsPrimitive() = default;

		const SharedRef<StaticMesh>& GetDebugMesh() const
		{
			return m_DebugMesh;
		}

	protected:
		void* m_InternalShape = nullptr;
		bool m_IsTrigger = false;
		SharedRef<StaticMesh> m_DebugMesh;
	};

	class SpherePhysicsPrimitive : public PhysicsPrimitive
	{
	public:
		SpherePhysicsPrimitive(const PhysicsBody& physicsBody, float radius);
		virtual ~SpherePhysicsPrimitive() = default;

	private:
		float m_Radius = 1.f;
	};

	class BoxPhysicsPrimitive : public PhysicsPrimitive
	{
	public:
		BoxPhysicsPrimitive(const PhysicsBody& physicsBody, glm::vec3 size);
		virtual ~BoxPhysicsPrimitive() = default;

	private:
		glm::vec3 m_Size = glm::vec3(1.f);
	};

	class CapsulePhysicsPrimitive : public PhysicsPrimitive
	{
	public:
		CapsulePhysicsPrimitive(const PhysicsBody& physicsBody, float radius, float height);
		virtual ~CapsulePhysicsPrimitive() = default;

	private:
		float m_Radius = 1.f;
		float m_Height = 1.f;
	};

	class ConvexPhysicsPrimitive : public PhysicsPrimitive
	{
	public:
		virtual ~ConvexPhysicsPrimitive() = default;
	};
} // namespace Neon
