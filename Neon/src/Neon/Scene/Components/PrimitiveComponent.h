#pragma once

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Renderer/Mesh.h"

namespace Neon
{
	class PrimitiveComponent
	{
	public:
		PrimitiveComponent() = default;
		PrimitiveComponent(const PrimitiveComponent& other) = default;
		~PrimitiveComponent() = default;

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
