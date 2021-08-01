#pragma once

#include "Neon/Renderer/Mesh.h"

namespace Neon
{
	class StaticMesh : public Mesh
	{
	public:
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec3 Binormal;
			uint32 MaterialIndex;
			glm::vec2 Texcoord;
		};

	public:
		StaticMesh(const std::string& filename);
		virtual ~StaticMesh() = default;

	private:
		std::vector<Vertex> m_Vertices;
	};
} // namespace Neon
