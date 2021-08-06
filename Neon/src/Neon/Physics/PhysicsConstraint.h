#pragma once

#include "Neon/Physics/PhysicsBody.h"

namespace Neon
{
	class PhysicsConstraint : public RefCounted
	{
	public:
		static SharedRef<PhysicsConstraint> Create(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1);

	public:
		PhysicsConstraint(const SharedRef<PhysicsBody>& body0, const SharedRef<PhysicsBody>& body1);
		virtual ~PhysicsConstraint() = default;

		virtual void* GetHandle() const = 0;

		virtual void Destroy() = 0;

	protected:
		SharedRef<PhysicsBody> m_Body0;
		SharedRef<PhysicsBody> m_Body1;
	};
} // namespace Neon
