#include "neopch.h"

#include "Mesh.h"
#include "Neon/Renderer/Renderer.h"
#include "Neon/Renderer/SceneRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>

namespace Neon
{
	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		void write(const char* message) override
		{
			NEO_CORE_ERROR("Assimp error: {0}", message);
		}
	};	

	static const uint32_t s_MeshImportFlags = aiProcess_CalcTangentSpace | // Create binormals/tangents just in case
											  aiProcess_Triangulate | // Make sure we're triangles
											  aiProcess_SortByPType | // Split meshes by primitive type
											  aiProcess_GenNormals | // Make sure we have legit normals
											  aiProcess_GenUVCoords | // Convert UVs if required
											  aiProcess_OptimizeMeshes | // Batch draws where possible
											  aiProcess_ValidateDataStructure; // Validation

	static glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
	{
		glm::mat4 result;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		result[0][0] = matrix.a1;
		result[1][0] = matrix.a2;
		result[2][0] = matrix.a3;
		result[3][0] = matrix.a4;
		result[0][1] = matrix.b1;
		result[1][1] = matrix.b2;
		result[2][1] = matrix.b3;
		result[3][1] = matrix.b4;
		result[0][2] = matrix.c1;
		result[1][2] = matrix.c2;
		result[2][2] = matrix.c3;
		result[3][2] = matrix.c4;
		result[0][3] = matrix.d1;
		result[1][3] = matrix.d2;
		result[2][3] = matrix.d3;
		result[3][3] = matrix.d4;
		return result;
	}

	Mesh::Mesh(const std::string& name, const std::vector<Index>& indices)
	{
		NEO_CORE_INFO("Generating mesh: {0}", name);

		NEO_CORE_ASSERT(!indices.empty());

		m_Indices = indices;

		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32>(m_Indices.size()) * sizeof(Index));

		Submesh& submesh = m_Submeshes.emplace_back();
		submesh.BaseVertex = 0;
		submesh.BaseIndex = 0;
		submesh.MaterialIndex = 0;
		submesh.IndexCount = static_cast<uint32>(indices.size()) * 3;
		submesh.MeshName = name;
	}

	Mesh::Mesh(const std::string& filename)
		: m_FilePath(filename)
	{
		LogStream::Initialize();

		NEO_CORE_INFO("Loading mesh: {0}", filename.c_str());

		m_Importer = CreateUnique<Assimp::Importer>();

		m_Scene = m_Importer->ReadFile(filename, s_MeshImportFlags);

		if (!m_Scene || !m_Scene->HasMeshes())
		{
			NEO_CORE_ERROR("Failed to load mesh file: {0}", filename);
			return;
		}

		m_InverseTransform = glm::inverse(Mat4FromAssimpMat4(m_Scene->mRootNode->mTransformation));

		uint32 vertexCount = 0;
		uint32 indexCount = 0;

		m_Submeshes.reserve(m_Scene->mNumMeshes);
		for (uint32 m = 0; m < m_Scene->mNumMeshes; m++)
		{
			aiMesh* mesh = m_Scene->mMeshes[m];
			NEO_CORE_ASSERT(mesh);

			Submesh& submesh = m_Submeshes.emplace_back();
			submesh.BaseVertex = vertexCount;
			submesh.BaseIndex = indexCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.MeshName = mesh->mName.C_Str();

			vertexCount += mesh->mNumVertices;
			indexCount += submesh.IndexCount;

			// Indices
			for (uint32 i = 0; i < mesh->mNumFaces; i++)
			{
				NEO_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.")
				Index index = {mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]};
				m_Indices.push_back(index);
			}
		}

		TraverseNodes(m_Scene->mRootNode);

		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32>(m_Indices.size()) * sizeof(Index));
	}

	Mesh::Mesh(ShaderSpecification& shaderSpec, GraphicsPipelineSpecification& pipelineSpec)
	{
		m_MeshShader = Shader::Create(shaderSpec);
		shaderSpec.ShaderPaths[ShaderType::Fragment] = "assets/shaders/Wireframe_Frag.glsl";
		m_WireframeMeshShader = Shader::Create(shaderSpec);
		m_MeshGraphicsPipeline = GraphicsPipeline::Create(m_MeshShader, pipelineSpec);
		pipelineSpec.Mode = PolygonMode::Line;
		m_WireframeMeshGraphicsPipeline = GraphicsPipeline::Create(m_WireframeMeshShader, pipelineSpec);
	}

	void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform /*= glm::mat4(1.0f)*/, uint32 level /*= 0*/)
	{
		glm::mat4 transform = parentTransform * Mat4FromAssimpMat4(node->mTransformation);
		for (uint32 i = 0; i < node->mNumMeshes; i++)
		{
			uint32 mesh = node->mMeshes[i];
			auto& submesh = m_Submeshes[mesh];
			submesh.NodeName = node->mName.C_Str();
			submesh.Transform = transform;
		}

		for (uint32 i = 0; i < node->mNumChildren; i++)
		{
			TraverseNodes(node->mChildren[i], transform, level + 1);
		}
	}

	void Mesh::CreateShaderAndGraphicsPipeline(ShaderSpecification& shaderSpecification, ShaderSpecification& wireframeShaderSpecification)
	{
		m_MeshShader = Shader::Create(shaderSpecification);
		m_WireframeMeshShader = Shader::Create(wireframeShaderSpecification);

		GraphicsPipelineSpecification graphicsPipelineSpecification;
		graphicsPipelineSpecification.Pass = SceneRenderer::GetGeoPass();
		m_MeshGraphicsPipeline = GraphicsPipeline::Create(m_MeshShader, graphicsPipelineSpecification);

		graphicsPipelineSpecification.Mode = PolygonMode::Line;
		m_WireframeMeshGraphicsPipeline = GraphicsPipeline::Create(m_WireframeMeshShader, graphicsPipelineSpecification);

		// Materials
		if (m_Scene && m_Scene->HasMaterials())
		{
			NEO_MESH_LOG("---- Materials - {0} ----", m_FilePath);

			m_Materials.resize(m_Scene->mNumMaterials);
			for (uint32 i = 0; i < m_Scene->mNumMaterials; i++)
			{
				m_Materials[i] = Material(i, m_MeshShader);

				MaterialProperties materialProperties;

				auto aiMaterial = m_Scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();

				NEO_MESH_LOG("  {0} (Index = {1})", aiMaterialName.data, i);
				aiString aiTexPath;
				uint32 textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
				NEO_MESH_LOG("    TextureCount = {0}", textureCount);

				aiColor3D aiColor;
				aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);

				float shininess, metalness;
				if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
				{
					shininess = 80.0f;
				}

				if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
				{
					metalness = 0.0f;
				}

				float roughness = 1.0f - glm::sqrt(shininess / 100.0f);

				NEO_MESH_LOG("    TextureCount = {0}", textureCount);
				("    COLOR = {0}, {1}, {2}", aiColor.r, aiColor.g, aiColor.b);
				NEO_MESH_LOG("    TextureCount = {0}", textureCount);
				("    ROUGHNESS = {0}", roughness);
				if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS)
				{
					std::filesystem::path path = m_FilePath;
					auto parentPath = path.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					NEO_MESH_LOG("    Albedo map path = {0}", texturePath);
					m_Materials[i].LoadTexture2D("u_AlbedoTextures", texturePath,
												 {TextureUsageFlagBits::ShaderRead, TextureFormat::SRGBA8}, 0);
					materialProperties.UseAlbedoMap = 1.f;
					NEO_MESH_LOG("    Texture {0} loaded", texturePath);
				}
				else
				{
					NEO_MESH_LOG("    No albedo map!");
					m_Materials[i].LoadDefaultTexture2D("u_AlbedoTextures", 0);
					materialProperties.AlbedoColor = glm::vec4{aiColor.r, aiColor.g, aiColor.b, 1.f};
					materialProperties.UseAlbedoMap = 0.f;
				}

				if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS)
				{
					std::filesystem::path path = m_FilePath;
					auto parentPath = path.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					m_Materials[i].LoadTexture2D("u_NormalTextures", texturePath, {TextureUsageFlagBits::ShaderRead}, 0);
					materialProperties.UseNormalMap = 1.f;
					NEO_MESH_LOG("    Normal map path = {0}", texturePath);
				}
				else
				{
					NEO_MESH_LOG("    No normal map!");
					m_Materials[i].LoadDefaultTexture2D("u_NormalTextures", 0);
					materialProperties.UseNormalMap = 0.f;
				}

				if (aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS)
				{
					std::filesystem::path path = m_FilePath;
					auto parentPath = path.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					NEO_MESH_LOG("    Roughness map path = {0}", texturePath);
					m_Materials[i].LoadTexture2D("u_RoughnessTextures", texturePath, {TextureUsageFlagBits::ShaderRead}, 0);
					materialProperties.UseRoughnessMap = 1.f;
				}
				else
				{
					NEO_MESH_LOG("    No roughness map");
					m_Materials[i].LoadDefaultTexture2D("u_RoughnessTextures", 0);
					materialProperties.Roughness = roughness;
					materialProperties.UseRoughnessMap = 0.f;
				}

				bool metalnessTextureFound = false;
				for (uint32 j = 0; j < aiMaterial->mNumProperties; j++)
				{
					auto prop = aiMaterial->mProperties[j];

					if (prop->mType == aiPTI_String)
					{
						uint32 strLength = *(uint32*)prop->mData;
						std::string str(prop->mData + 4, strLength);

						std::string key = prop->mKey.data;
						if (key == "$raw.ReflectionFactor|file")
						{
							metalnessTextureFound = true;

							std::filesystem::path path = m_FilePath;
							auto parentPath = path.parent_path();
							parentPath /= str;
							std::string texturePath = parentPath.string();
							NEO_MESH_LOG("    Metalness map path = {0}", texturePath);
							m_Materials[i].LoadTexture2D("u_MetalnessTextures", texturePath, {TextureUsageFlagBits::ShaderRead}, 0);
							materialProperties.UseMetalnessMap = 1.f;
							break;
						}
					}
				}

				if (!metalnessTextureFound)
				{
					NEO_MESH_LOG("    No metalness map");
					m_Materials[i].LoadDefaultTexture2D("u_MetalnessTextures", 0);
					materialProperties.Metalness = metalness;
					materialProperties.UseMetalnessMap = 0.f;
				}
				NEO_MESH_LOG("------------------------");

				m_Materials[i].SetProperties(materialProperties);
			}
		}
		else
		{
			NEO_MESH_LOG("---- Creating Default Material ----");

			m_Materials.resize(1);
			m_Materials[0] = Material(0, m_MeshShader);

			MaterialProperties materialProperties;

			float shininess = 80.f;
			float metalness = 0.f;
			float roughness = 1.0f - glm::sqrt(shininess / 100.f);

			m_Materials[0].LoadDefaultTexture2D("u_AlbedoTextures", 0);
			materialProperties.AlbedoColor = glm::vec4{1.f};
			materialProperties.UseAlbedoMap = 0.f;

			m_Materials[0].LoadDefaultTexture2D("u_NormalTextures", 0);
			materialProperties.UseNormalMap = 0.f;

			m_Materials[0].LoadDefaultTexture2D("u_RoughnessTextures", 0);
			materialProperties.Roughness = roughness;
			materialProperties.UseRoughnessMap = 0.f;

			m_Materials[0].LoadDefaultTexture2D("u_MetalnessTextures", 0);
			materialProperties.Metalness = metalness;
			materialProperties.UseMetalnessMap = 0.f;

			NEO_MESH_LOG("------------------------");

			m_Materials[0].SetProperties(materialProperties);
		}
		

		m_MeshShader->SetTextureCube("u_EnvRadianceTex", 0, SceneRenderer::GetRadianceTex(), 0);
		m_MeshShader->SetTextureCube("u_EnvIrradianceTex", 0, SceneRenderer::GetIrradianceTex(), 0);
		m_MeshShader->SetTexture2D("u_BRDFLUTTexture", 0, SceneRenderer::GetBRDFLUTTex(), 0);
	}

} // namespace Neon
