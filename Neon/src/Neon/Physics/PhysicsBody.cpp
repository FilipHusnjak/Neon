#include "neopch.h"

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Renderer/SceneRenderer.h"

namespace Neon
{
	PhysicsBody::PhysicsBody(PhysicsBodyType bodyType, const Transform& transform, const SharedRef<PhysicsMaterial>& material)
		: m_BodyType(bodyType)
		, m_Material(material)
	{
		if (!m_Material)
		{
			m_Material = PhysicsMaterial::CreateMaterial();
		}
	}

	void PhysicsBody::SetMaterial(const SharedRef<PhysicsMaterial>& material)
	{
		m_Material = material;
	}

	void PhysicsBody::RenderCollision() const
	{
		glm::mat4 transformMat = GetBodyTransform().GetMatrix();
		for (const auto& sphere : m_Spheres)
		{
			SceneRenderer::SubmitMesh(sphere->GetDebugMesh(), sphere->GetTransform().GetMatrix() * transformMat, true);
		}
		for (const auto& box : m_Boxes)
		{
			SceneRenderer::SubmitMesh(box->GetDebugMesh(), box->GetTransform().GetMatrix() * transformMat, true);
		}
		for (const auto& capsule : m_Capsules)
		{
			SceneRenderer::SubmitMesh(capsule->GetDebugMesh(), capsule->GetTransform().GetMatrix() * transformMat, true);
		}
	}

} // namespace Neon
