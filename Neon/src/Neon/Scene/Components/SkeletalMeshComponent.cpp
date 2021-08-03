#include "neopch.h"

#include "Neon/Physics/Physics.h"
#include "Neon/Renderer/RendererContext.h"
#include "Neon/Scene/Components/SkeletalMeshComponent.h"

namespace Neon
{
	SkeletalMeshComponent::SkeletalMeshComponent(Actor* owner, const SharedRef<SkeletalMesh>& skeletalMesh)
		: PrimitiveComponent(owner)
		, m_SkeletalMesh(skeletalMesh)
	{
	}

	SkeletalMeshComponent::~SkeletalMeshComponent()
	{
		RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(m_SkeletalMesh));
	}

	void SkeletalMeshComponent::CreatePhysicsBody(const std::string& boneName /*= std::string()*/)
	{
		if (m_PhysicsBodyMap.find(boneName) != m_PhysicsBodyMap.end())
		{
			SharedRef<PhysicsBody> body = m_PhysicsBodyMap[boneName];
			if (body)
			{
				Physics::GetCurrentScene()->RemovePhysicsBody(body);
			}
		}
		m_PhysicsBodyMap[boneName] = Physics::GetCurrentScene()->AddPhysicsBody();
	}

	void SkeletalMeshComponent::TickComponent(float deltaSeconds)
	{
		if (m_SkeletalMesh)
		{
			m_SkeletalMesh->TickAnimation(deltaSeconds);
		}
	}

	void SkeletalMeshComponent::LoadMesh(const std::string& filename)
	{
		m_SkeletalMesh = SharedRef<SkeletalMesh>::Create(filename);
	}

} // namespace Neon
