#include "neopch.h"

#include "Mesh.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>

namespace Neon
{
#define MESH_DEBUG_LOG 1
#if MESH_DEBUG_LOG
	#define NEO_MESH_LOG(...) NEO_CORE_TRACE(__VA_ARGS__)
#else
	#define NEO_MESH_LOG(...)
#endif

	glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
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

	static const uint32_t s_MeshImportFlags = aiProcess_CalcTangentSpace | // Create binormals/tangents just in case
											  aiProcess_Triangulate | // Make sure we're triangles
											  aiProcess_SortByPType | // Split meshes by primitive type
											  aiProcess_GenNormals | // Make sure we have legit normals
											  aiProcess_GenUVCoords | // Convert UVs if required
											  aiProcess_OptimizeMeshes | // Batch draws where possible
											  aiProcess_ValidateDataStructure; // Validation

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
		}

		std::unordered_map<ShaderType, std::string> shaderPaths;
		shaderPaths[ShaderType::Vertex] = "assets\\shaders\\test.vert";
		shaderPaths[ShaderType::Fragment] = "assets\\shaders\\test.frag";
		ShaderSpecification shaderSpecification;
		shaderSpecification.ShaderVariableCounts["u_AlbedoTexture"] = m_Scene->mNumMaterials;
		m_MeshShader = Shader::Create(shaderSpecification, shaderPaths);

		uint32 vertexCount = 0;
		uint32 indexCount = 0;

		m_Submeshes.reserve(m_Scene->mNumMeshes);
		for (uint32 m = 0; m < m_Scene->mNumMeshes; m++)
		{
			aiMesh* mesh = m_Scene->mMeshes[m];

			Submesh& submesh = m_Submeshes.emplace_back();
			submesh.BaseVertex = vertexCount;
			submesh.BaseIndex = indexCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.MeshName = mesh->mName.C_Str();

			vertexCount += mesh->mNumVertices;
			indexCount += submesh.IndexCount;

			NEO_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");
			NEO_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");

			// Vertices
			for (uint32 i = 0; i < mesh->mNumVertices; i++)
			{
				StaticVertex vertex;
				vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
				vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};

				if (mesh->HasTangentsAndBitangents())
				{
					vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
					vertex.Binormal = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
				}

				vertex.MaterialIndex = submesh.MaterialIndex;
				if (mesh->HasTextureCoords(0))
				{
					vertex.Texcoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
				}

				m_StaticVertices.push_back(vertex);
			}

			// Indices
			for (uint32 i = 0; i < mesh->mNumFaces; i++)
			{
				NEO_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.")
				Index index = {mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2]};
				m_Indices.push_back(index);
			}
		}

		TraverseNodes(m_Scene->mRootNode);

		m_VertexBufferLayout = {{ShaderDataType::Float3}, {ShaderDataType::Float3}, {ShaderDataType::Float3},
								{ShaderDataType::Float3}, {ShaderDataType::UInt},	{ShaderDataType::Float2}};
		m_VertexBuffer = VertexBuffer::Create(
			m_StaticVertices.data(), static_cast<uint32>(m_StaticVertices.size()) * sizeof(StaticVertex), m_VertexBufferLayout);

		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32>(m_Indices.size()) * sizeof(Index));

		// Materials
		if (m_Scene->HasMaterials())
		{
			NEO_MESH_LOG("---- Materials - {0} ----", filename);

			m_Textures.resize(m_Scene->mNumMaterials);
			for (uint32 i = 0; i < m_Scene->mNumMaterials; i++)
			{
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
					std::filesystem::path path = filename;
					auto parentPath = path.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					NEO_MESH_LOG("    Albedo map path = {0}", texturePath);
					auto texture = Texture2D::Create(texturePath, true);
					if (texture->Loaded())
					{
						m_Textures[i] = texture;
						m_MeshShader->SetTexture(1, i, texture);
						NEO_MESH_LOG("    Texture {0} loaded", texturePath);
					}
					else
					{
						//mi->Set("u_MaterialUniforms.AlbedoColor", glm::vec3 { aiColor.r, aiColor.g, aiColor.b });
						NEO_CORE_ERROR("Could not load texture: {0}", texturePath);
					}
				}
				else
				{
					//mi->Set("u_MaterialUniforms.AlbedoColor", glm::vec3 { aiColor.r, aiColor.g, aiColor.b });
					NEO_MESH_LOG("    No albedo map");
				}

				if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS)
				{
					std::filesystem::path path = filename;
					auto parentPath = path.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					NEO_MESH_LOG("    Normal map path = {0}", texturePath);
				}
				else
				{
					NEO_MESH_LOG("    No normal map");
				}

				// Roughness map
				// mi->Set("u_Roughness", 1.0f);
				// mi->Set("u_RoughnessTexToggle", 0.0f);
				if (aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS)
				{
					// TODO: Temp - this should be handled by Hazel's filesystem
					std::filesystem::path path = filename;
					auto parentPath = path.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();
					NEO_MESH_LOG("    Roughness map path = {0}", texturePath);
				}
				else
				{
					NEO_MESH_LOG("    No roughness map");
					//mi->Set("u_MaterialUniforms.Roughness", roughness);
				}

				bool metalnessTextureFound = false;
				for (uint32_t i = 0; i < aiMaterial->mNumProperties; i++)
				{
					auto prop = aiMaterial->mProperties[i];

					if (false && prop->mType == aiPTI_String)
					{
						uint32_t strLength = *(uint32_t*)prop->mData;
						std::string str(prop->mData + 4, strLength);

						std::string key = prop->mKey.data;
						if (key == "$raw.ReflectionFactor|file")
						{
							metalnessTextureFound = true;

							std::filesystem::path path = filename;
							auto parentPath = path.parent_path();
							parentPath /= str;
							std::string texturePath = parentPath.string();
							NEO_MESH_LOG("    Metalness map path = {0}", texturePath);
							break;
						}
					}
				}

				if (!metalnessTextureFound)
				{
					NEO_MESH_LOG("    No metalness map");

					//mi->Set("u_MaterialUniforms.Metalness", metalness);
					//mi->Set("u_MaterialUniforms.MetalnessTexToggle", 0.0f);
				}
				NEO_MESH_LOG("------------------------");
			}
		}
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

} // namespace Neon
