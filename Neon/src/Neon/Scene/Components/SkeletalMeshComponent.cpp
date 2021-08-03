#include "neopch.h"

#include "Neon/Renderer/RendererContext.h"
#include "Neon/Scene/Components/SkeletalMeshComponent.h"

namespace Neon
{
	SkeletalMeshComponent::SkeletalMeshComponent(const SharedRef<SkeletalMesh>& skeletalMesh)
		: m_SkeletalMesh(skeletalMesh)
	{
	}

	SkeletalMeshComponent::~SkeletalMeshComponent()
	{
		RendererContext::Get()->SafeDeleteResource(StaleResourceWrapper::Create(m_SkeletalMesh));
	}

	void SkeletalMeshComponent::OnUpdate(float deltaSeconds)
	{
		if (m_SkeletalMesh)
		{
			m_SkeletalMesh->OnUpdate(deltaSeconds);
		}
	}

	void SkeletalMeshComponent::LoadMesh(const std::string& filename)
	{
		m_SkeletalMesh = SharedRef<SkeletalMesh>::Create(filename);
	}

} // namespace Neon
