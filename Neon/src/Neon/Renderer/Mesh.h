#pragma once

#include "Neon/Renderer/IndexBuffer.h"
#include "Neon/Renderer/Material.h"
#include "Neon/Renderer/Pipeline.h"
#include "Neon/Renderer/Texture.h"
#include "Neon/Renderer/VertexBuffer.h"

#include <glm/glm.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;
struct aiMesh;

namespace Assimp
{
	class Importer;
}

#define MESH_DEBUG_LOG 1
#if MESH_DEBUG_LOG
	#define NEO_MESH_LOG(...) NEO_CORE_TRACE(__VA_ARGS__)
#else
	#define NEO_MESH_LOG(...)
#endif

namespace Neon
{
	struct Index
	{
		uint32 V1, V2, V3;
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
		//static SharedRef<Mesh> GenerateGridMesh(uint32 countW, uint32 countH, ShaderSpecification& shaderSpec,
		//										GraphicsPipelineSpecification& pipelineSpec);

		Mesh(const std::string& filename);
		virtual ~Mesh() = default;

		const SharedRef<GraphicsPipeline>& GetGraphicsPipeline() const
		{
			return m_MeshGraphicsPipeline;
		}
		const SharedRef<GraphicsPipeline>& GetWireframeGraphicsPipeline() const
		{
			return m_WireframeMeshGraphicsPipeline;
		}

		const SharedRef<Shader>& GetShader() const
		{
			return m_MeshShader;
		}
		const SharedRef<Shader>& GetWireframeShader() const
		{
			return m_WireframeMeshShader;
		}

		const SharedRef<VertexBuffer>& GetVertexBuffer() const
		{
			return m_VertexBuffer;
		}
		const SharedRef<IndexBuffer>& GetIndexBuffer() const
		{
			return m_IndexBuffer;
		}

		const std::vector<Submesh>& GetSubmeshes() const
		{
			return m_Submeshes;
		}

		std::vector<Material>& GetMaterials()
		{
			return m_Materials;
		}
		const std::vector<Material>& GetMaterials() const
		{
			return m_Materials;
		}

		const std::string& GetFilePath() const
		{
			return m_FilePath;
		}

	protected:
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32 level = 0);
		void CreateShaderAndGraphicsPipeline(ShaderSpecification& shaderSpecification,
											 ShaderSpecification& wireframeShaderSpecification);

	protected:
		const aiScene* m_Scene;
		std::vector<Submesh> m_Submeshes;

		UniqueRef<Assimp::Importer> m_Importer;

		glm::mat4 m_InverseTransform;

		SharedRef<VertexBuffer> m_VertexBuffer;
		SharedRef<IndexBuffer> m_IndexBuffer;

		std::vector<Index> m_Indices;

		SharedRef<Shader> m_MeshShader;
		SharedRef<Shader> m_WireframeMeshShader;
		SharedRef<GraphicsPipeline> m_MeshGraphicsPipeline;
		SharedRef<GraphicsPipeline> m_WireframeMeshGraphicsPipeline;

		std::vector<Material> m_Materials;

		std::string m_FilePath;
	};
} // namespace Neon
