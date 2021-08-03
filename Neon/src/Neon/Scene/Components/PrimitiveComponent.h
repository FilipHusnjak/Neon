#pragma once

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Scene/Components/ActorComponent.h"

namespace Neon
{
	class Actor;

	class PrimitiveComponent : public ActorComponent
	{
	public:
		PrimitiveComponent() = default;
		PrimitiveComponent(Actor* owner);
		PrimitiveComponent(const PrimitiveComponent& other) = default;
		virtual ~PrimitiveComponent() = default;

		virtual void CreatePhysicsBody(const std::string& boneName = std::string());

	private:
		bool m_LockPositionX = false;
		bool m_LockPositionY = false;
		bool m_LockPositionZ = false;
		bool m_LockRotationX = false;
		bool m_LockRotationY = false;
		bool m_LockRotationZ = false;

		SharedRef<PhysicsBody> m_PhysicsBody;
	};
} // namespace Neon
