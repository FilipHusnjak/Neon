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
		m_PhysicsBodyMap[boneName] = Physics::GetCurrentScene()->AddPhysicsBody(bodyType, m_Owner->GetTransform());

		SceneRenderer::SubmitMesh(m_SkeletalMesh, m_Owner->GetTransform().GetMatrix());
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
