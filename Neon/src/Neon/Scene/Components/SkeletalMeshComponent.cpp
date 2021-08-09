#include "neopch.h"

#include "Neon/Physics/Physics.h"
#include "Neon/Renderer/RendererContext.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Components/SkeletalMeshComponent.h"

#include <glm/gtx/matrix_decompose.hpp>

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

	void SkeletalMeshComponent::CreatePhysicsBody(PhysicsBodyType bodyType, const std::string& boneName /*= std::string()*/,
												  const SharedRef<PhysicsMaterial>& material)
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

		if (boneName.empty())
		{
			m_RootPhysicsBody = Physics::GetCurrentScene()->AddPhysicsBody(bodyType, m_Owner->GetTransform(), material);
		}
		else
		{
			const SkeletalMesh::BoneInfo& boneInfo = m_SkeletalMesh->GetBoneInfo(boneName);
			Transform transform;
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(boneInfo.NodeTransform, scale, rotation, translation, skew, perspective);
			transform.Translation = translation;
			transform.Rotation = rotation;
			m_PhysicsBodyMap[boneName] = Physics::GetCurrentScene()->AddPhysicsBody(bodyType, transform * m_Owner->GetTransform(), material);

			if (m_RootPhysicsBody)
			{
				auto constraint = PhysicsConstraint::Create(m_PhysicsBodyMap[boneName], m_RootPhysicsBody);
				constraint->SetMotion(MotionAxis::RotationY, Motion::Free);
				constraint->SetMotion(MotionAxis::TranslationZ, Motion::Limited);
				constraint->SetDrive(DriveAxis::TranslationZ, 1000000.f, 0.1f, 1000000.f);
				constraint->SetLinearLimit(0.08f);
				constraint->SetDrivePosition(Transform());
				constraint->SetDriveVelocity(glm::vec3(), glm::vec3());
			}
		}
	}

	void SkeletalMeshComponent::TickComponent(float deltaSeconds)
	{
		PrimitiveComponent::TickComponent(deltaSeconds);

		if (m_SkeletalMesh)
		{
			m_SkeletalMesh->TickAnimation(deltaSeconds);

			if (m_RootPhysicsBody)
			{
				m_Owner->SetTranslation(m_RootPhysicsBody->GetBodyTranslation());
				m_Owner->SetRotation(m_RootPhysicsBody->GetBodyRotation());
			}

			for (const auto& [boneName, physicsBody] : m_PhysicsBodyMap)
			{
				Transform localTransform = physicsBody->GetBodyTransform() * m_Owner->GetTransform().Inverse();
				SkeletalMesh::BoneInfo& boneInfo = m_SkeletalMesh->GetBoneInfo(boneName);
				boneInfo.NodeTransform = localTransform.GetMatrix();

				if (SceneRenderer::GetSelectedActor() == m_Owner)
				{
					physicsBody->RenderCollision();
				}
			}

			SceneRenderer::SubmitMesh(m_SkeletalMesh, m_Owner->GetTransform().GetMatrix());
		}

		AddForceLocal(1000.f * glm::vec3(0.f, 0.f, 1.f));
	}

	SharedRef<PhysicsBody> SkeletalMeshComponent::GetPhysicsBody(const std::string& boneName) const
	{
		if (boneName.empty())
		{
			return m_RootPhysicsBody;
		}
		
		NEO_CORE_ASSERT(m_PhysicsBodyMap.find(boneName) != m_PhysicsBodyMap.end());
		return m_PhysicsBodyMap.at(boneName);
	}

	void SkeletalMeshComponent::LoadMesh(const std::string& filename)
	{
		m_SkeletalMesh = SharedRef<SkeletalMesh>::Create(filename);
	}

} // namespace Neon
