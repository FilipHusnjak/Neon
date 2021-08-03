#include "neopch.h"

#include "Neon/Renderer/RendererContext.h"
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

	void StaticMeshComponent::LoadMesh(const std::string& filename)
	{
		m_StaticMesh = SharedRef<StaticMesh>::Create(filename);
	}

} // namespace Neon
