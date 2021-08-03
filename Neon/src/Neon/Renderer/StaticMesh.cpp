#include "neopch.h"

#include "StaticMesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Neon
{
	StaticMesh::StaticMesh(const std::string& filename)
		: Mesh(filename)
	{
		for (uint32 m = 0; m < m_Scene->mNumMeshes; m++)
		{
			aiMesh* mesh = m_Scene->mMeshes[m];

			NEO_CORE_ASSERT(mesh);
			NEO_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");
			NEO_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");

			for (uint32 i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
				vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};

				if (mesh->HasTangentsAndBitangents())
				{
					vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
					vertex.Binormal = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
				}

				vertex.MaterialIndex = mesh->mMaterialIndex;
				if (mesh->HasTextureCoords(0))
				{
					vertex.Texcoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
				}

				m_Vertices.push_back(vertex);
			}
		}

		ShaderSpecification shaderSpecification;
		shaderSpecification.ShaderPaths[ShaderType::Fragment] = "assets/shaders/Pbr_Frag.glsl";
		shaderSpecification.ShaderPaths[ShaderType::Vertex] = "assets/shaders/PbrStatic_Vert.glsl";
		shaderSpecification.ShaderVariableCounts["MaterialUBO"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_AlbedoTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_NormalTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_RoughnessTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_MetalnessTextures"] = m_Scene->mNumMaterials;

		std::vector<VertexBufferElement> elements = {{ShaderDataType::Float3}, {ShaderDataType::Float3}, {ShaderDataType::Float3},
													 {ShaderDataType::Float3}, {ShaderDataType::UInt},	 {ShaderDataType::Float2}};

		VertexBufferLayout vertexBufferLayout = elements;
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), static_cast<uint32>(m_Vertices.size()) * sizeof(Vertex),
											  vertexBufferLayout);
		
		shaderSpecification.VBLayout = vertexBufferLayout;

		CreateShaderAndGraphicsPipeline(shaderSpecification);
	}

} // namespace Neon