#include "neopch.h"

#include "Neon/Physics/Physics.h"
#include "Neon/Renderer/RendererContext.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Actor.h"
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

	void SkeletalMeshComponent::CreatePhysicsBody(PhysicsBodyType bodyType, const std::string& boneName /*= std::string()*/)
	{
		NEO_CORE_ASSERT(m_Owner);

		if (m_PhysicsBodyMap.find(boneName) != m_PhysicsBodyMap.end())
		{
			SharedRef<PhysicsBody> body = m_PhysicsBodyMap[boneName];
			if (body)
			{
				Physics::GetCurrentScene()->RemovePhysicsBody(body);
			}
		}

		if (m_SkeletalMesh)
		{
			if (m_RootPhysicsBody)
			{
				m_Owner->SetTranslation(m_RootPhysicsBody->GetBodyTranslation());
				m_Owner->SetRotation(m_RootPhysicsBody->GetBodyRotation());
			}

			for (const auto& [boneName, physicsBody] : m_PhysicsBodyMap)
			{
				
			}
		}

		if (boneName.empty())
		{
			m_RootPhysicsBody = Physics::GetCurrentScene()->AddPhysicsBody(bodyType, m_Owner->GetTransform());
		}
		else
		{
			m_PhysicsBodyMap[boneName] = Physics::GetCurrentScene()->AddPhysicsBody(bodyType, m_Owner->GetTransform());
		}
	}

	void SkeletalMeshComponent::TickComponent(float deltaSeconds)
	{
		PrimitiveComponent::TickComponent(deltaSeconds);

		if (m_SkeletalMesh)
		{
			m_SkeletalMesh->TickAnimation(deltaSeconds);
		}

		if (m_SkeletalMesh)
		{
			SceneRenderer::SubmitMesh(m_SkeletalMesh, m_Owner->GetTransform().GetMatrix());
		}
	}

	void SkeletalMeshComponent::LoadMesh(const std::string& filename)
	{
		m_SkeletalMesh = SharedRef<SkeletalMesh>::Create(filename);
	}

} // namespace Neon
