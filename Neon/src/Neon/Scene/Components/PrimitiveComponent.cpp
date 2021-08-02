#include "neopch.h"

#include "Neon/Physics/Physics.h"
#include "Neon/Scene/Components/PrimitiveComponent.h"

namespace Neon
{
	void PrimitiveComponent::CreatePhysicsBody(const std::string& boneName /*= std::string()*/)
	{
		if (m_PhysicsBody)
		{
			Physics::GetCurrentScene()->RemovePhysicsBody(m_PhysicsBody);
		}
		m_PhysicsBody = Physics::GetCurrentScene()->AddPhysicsBody();
	}

} // namespace Neon
