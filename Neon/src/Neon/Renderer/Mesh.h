#pragma once

#include "IndexBuffer.h"
#include "Pipeline.h"
#include "VertexBuffer.h"

#include <glm/glm.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp
{
	class Importer;
}

namespace Neon
{
	struct StaticVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 Texcoord;
	};

	struct Index
	{
		uint32 V1, V2, V3;
	};

	struct Triangle
	{
		StaticVertex V0, V1, V2;

		Triangle(const StaticVertex& v0, const StaticVertex& v1, const StaticVertex& v2)
			: V0(v0)
			, V1(v1)
			, V2(v2)
		{
		}
	};

	class Submesh
	{
	public:
		uint32 BaseVertex;
		uint32 BaseIndex;
		uint32 MaterialIndex;
		uint32 IndexCount;

		glm::mat4 Transform;

		std::string NodeName;
		std::string MeshName;
	};

	class Mesh : public RefCounted
	{
	public:
		Mesh(const std::string& filename);
		~Mesh();

	private:
		std::vector<Submesh> m_Submeshes;
		UniqueRef<Assimp::Importer> m_Importer;

		glm::mat4 m_InverseTransform;

		SharedRef<Pipeline> m_Pipeline;
		SharedRef<VertexBuffer> m_VertexBuffer;
		SharedRef<IndexBuffer> m_IndexBuffer;
		VertexBufferLayout m_VertexBufferLayout;

		std::vector<StaticVertex> m_StaticVertices;
		std::vector<Index> m_Indices;
		const aiScene* m_Scene;

		SharedRef<Shader> m_MeshShader;

		std::string m_FilePath;
	};
} // namespace Neon
