#include "neopch.h"

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Renderer/SceneRenderer.h"

namespace Neon
{
	PhysicsBody::PhysicsBody(PhysicsBodyType bodyType, const Transform& transform)
		: m_BodyType(bodyType)
	{
	}

	void PhysicsBody::SetMaterial(const SharedRef<PhysicsMaterial>& material)
	{
		m_Material = material;
	}

	void PhysicsBody::RenderCollision()
	{
		glm::mat4 transformMat = GetBodyTransform().GetMatrix();
		for (const auto& sphere : m_Spheres)
		{
			SceneRenderer::SubmitMesh(sphere->GetDebugMesh(), transformMat, true);
		}
		for (const auto& box : m_Boxes)
		{
			SceneRenderer::SubmitMesh(box->GetDebugMesh(), transformMat, true);
		}
		for (const auto& capsule : m_Capsules)
		{
			SceneRenderer::SubmitMesh(capsule->GetDebugMesh(), transformMat, true);
		}
	}

} // namespace Neon
