#include "neopch.h"

#include "Neon/Core/Application.h"
#include "Neon/Renderer/RendererContext.h"
#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Scene/Actor.h"
#include "Neon/Scene/Components/StaticMeshComponent.h"

namespace Neon
{
	StaticMeshComponent::StaticMeshComponent(Actor* owner, const SharedRef<StaticMesh>& staticMesh)
		: PrimitiveComponent(owner)
		, m_StaticMesh(staticMesh)
	{
	}

	StaticMeshComponent::~StaticMeshComponent()
	{
		RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(m_StaticMesh));
	}

	void StaticMeshComponent::TickComponent(float deltaSeconds)
	{
		PrimitiveComponent::TickComponent(deltaSeconds);

		if (m_RootPhysicsBody)
		{
			m_Owner->SetTranslation(m_RootPhysicsBody->GetBodyTranslation());
			m_Owner->SetRotation(m_RootPhysicsBody->GetBodyRotation());
		}

		if (m_StaticMesh)
		{
			SceneRenderer::SubmitMesh(m_StaticMesh, m_Owner->GetTransform().GetMatrix());
		}
	}

	void StaticMeshComponent::LoadMesh(const std::string& filename)
	{
		m_StaticMesh = SharedRef<StaticMesh>::Create(filename);
	}

} // namespace Neon
