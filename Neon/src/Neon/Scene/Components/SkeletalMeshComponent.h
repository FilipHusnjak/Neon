#pragma once

#include "Neon/Renderer/SkeletalMesh.h"
#include "Neon/Scene/Components/MeshComponent.h"

namespace Neon
{
	class SkeletalMeshComponent : public MeshComponent
	{
	public:
		SkeletalMeshComponent() = default;
		SkeletalMeshComponent(const SkeletalMeshComponent& other) = default;
		SkeletalMeshComponent(const SharedRef<SkeletalMesh>& skeletalMesh);
		~SkeletalMeshComponent();

		void OnUpdate(float deltaSeconds);

		void LoadMesh(const std::string& filename);

		SharedRef<SkeletalMesh> GetMesh() const
		{
			return m_SkeletalMesh;
		}

	private:
		SharedRef<SkeletalMesh> m_SkeletalMesh;
	};
} // namespace Neon
