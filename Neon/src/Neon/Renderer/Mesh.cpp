#include "neopch.h"

#include "Mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

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

	Mesh::Mesh(const std::string& filename, const SharedRef<RenderPass>& renderPass)
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

		m_IsAnimated = m_Scene->mAnimations != nullptr;

		m_InverseTransform = glm::inverse(Mat4FromAssimpMat4(m_Scene->mRootNode->mTransformation));

		ShaderSpecification shaderSpecification;
		shaderSpecification.ShaderPaths[ShaderType::Fragment] = "assets/shaders/Pbr_Frag.glsl";
		if (m_IsAnimated)
		{
			shaderSpecification.ShaderPaths[ShaderType::Vertex] = "assets/shaders/PbrAnim_Vert.glsl";
		}
		else
		{
			shaderSpecification.ShaderPaths[ShaderType::Vertex] = "assets/shaders/PbrStatic_Vert.glsl";
		}

		shaderSpecification.ShaderVariableCounts["MaterialUBO"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_AlbedoTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_NormalTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_RoughnessTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_MetalnessTextures"] = m_Scene->mNumMaterials;

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

			if (m_IsAnimated)
			{
				// Vertices
				for (uint32 i = 0; i < mesh->mNumVertices; i++)
				{
					AnimatedVertex vertex;
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

					m_AnimatedVertices.push_back(vertex);
				}
			}
			else
			{
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

		// Bones
		if (m_IsAnimated)
		{
			for (uint32 m = 0; m < m_Scene->mNumMeshes; m++)
			{
				aiMesh* mesh = m_Scene->mMeshes[m];
				Submesh& submesh = m_Submeshes[m];

				for (uint32 i = 0; i < mesh->mNumBones; i++)
				{
					aiBone* bone = mesh->mBones[i];
					std::string boneName(bone->mName.data);
					uint32 boneIndex = 0;

					if (m_BoneMapping.find(boneName) == m_BoneMapping.end())
					{
						// Allocate an index for a new bone
						boneIndex = static_cast<uint32>(m_BoneInfo.size());
						BoneInfo bi;
						m_BoneInfo.push_back(bi);
						m_BoneInfo[boneIndex].BoneOffset = Mat4FromAssimpMat4(bone->mOffsetMatrix);
						m_BoneMapping[boneName] = boneIndex;
					}
					else
					{
						NEO_MESH_LOG("Found existing bone in map");
						boneIndex = m_BoneMapping[boneName];
					}

					for (uint32 j = 0; j < bone->mNumWeights; j++)
					{
						uint32 vertexId = submesh.BaseVertex + bone->mWeights[j].mVertexId;
						float weight = bone->mWeights[j].mWeight;
						m_AnimatedVertices[vertexId].AddBoneData(boneIndex, weight);
					}
				}
			}
		}

		std::vector<VertexBufferElement> elements = {{ShaderDataType::Float3}, {ShaderDataType::Float3}, {ShaderDataType::Float3},
													 {ShaderDataType::Float3}, {ShaderDataType::UInt},	 {ShaderDataType::Float2}};
		VertexBufferLayout vertexBufferLayout;
		if (m_IsAnimated)
		{
			for (uint32 i = 0; i < MAX_BONES_PER_VERTEX; i++)
			{
				elements.emplace_back(ShaderDataType::UInt);
			}
			for (uint32 i = 0; i < MAX_BONES_PER_VERTEX; i++)
			{
				elements.emplace_back(ShaderDataType::Float);
			}
			vertexBufferLayout = elements;
			m_VertexBuffer =
				VertexBuffer::Create(m_AnimatedVertices.data(),
									 static_cast<uint32>(m_AnimatedVertices.size()) * sizeof(AnimatedVertex), vertexBufferLayout);
		}
		else
		{
			vertexBufferLayout = elements;
			m_VertexBuffer = VertexBuffer::Create(
				m_StaticVertices.data(), static_cast<uint32>(m_StaticVertices.size()) * sizeof(StaticVertex), vertexBufferLayout);
		}

		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), static_cast<uint32>(m_Indices.size()) * sizeof(Index));

		shaderSpecification.VBLayout = vertexBufferLayout;
		shaderSpecification.ShaderVariableCounts["BonesUBO"] = static_cast<uint32>(m_BoneInfo.size());
		m_MeshShader = Shader::Create(shaderSpecification);

		GraphicsPipelineSpecification graphicsPipelineSpecification;
		graphicsPipelineSpecification.Shader = m_MeshShader;
		graphicsPipelineSpecification.Pass = renderPass;
		m_MeshGraphicsPipeline = GraphicsPipeline::Create(graphicsPipelineSpecification);

		// Materials
		if (m_Scene->HasMaterials())
		{
			NEO_MESH_LOG("---- Materials - {0} ----", filename);

			m_Materials.resize(m_Scene->mNumMaterials, SharedRef<Material>::Create(m_MeshShader));
			for (uint32 i = 0; i < m_Scene->mNumMaterials; i++)
			{
				struct
				{
					glm::vec4 AlbedoColor;
					float HasAlbedoTexture;
					float HasNormalTex;
					float Metalness;
					float HasMetalnessTex;
					float Roughness;
					float HasRoughnessTex;
				} materialProperties;

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
					m_Materials[i]->LoadTexture2D("u_AlbedoTextures", i, texturePath, {TextureType::SRGB});
					materialProperties.HasAlbedoTexture = 1.f;
					NEO_MESH_LOG("    Texture {0} loaded", texturePath);
				}
				else
				{
					NEO_MESH_LOG("    No albedo map!");
					m_Materials[i]->LoadDefaultTexture2D("u_AlbedoTextures", i);
					materialProperties.AlbedoColor = glm::vec4{aiColor.r, aiColor.g, aiColor.b, 1.f};
					materialProperties.HasAlbedoTexture = 0.f;
				}

				if (true || aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS)
				{
					std::filesystem::path path = filename;
					auto parentPath = path.parent_path();
					parentPath /= std::string(aiTexPath.data);
					//std::string texturePath = parentPath.string();
					std::string texturePath = "assets/models/cerberus/textures/Cerberus_N.tga";
					m_Materials[i]->LoadTexture2D("u_NormalTextures", i, texturePath, {});
					materialProperties.HasNormalTex = 1.f;
					NEO_MESH_LOG("    Normal map path = {0}", texturePath);
				}
				else
				{
					m_Materials[i]->LoadDefaultTexture2D("u_NormalTextures", i);
					materialProperties.HasNormalTex = 0.f;
					NEO_MESH_LOG("    No normal map!");
				}

				if (true || aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS)
				{
					std::filesystem::path path = filename;
					auto parentPath = path.parent_path();
					parentPath /= std::string(aiTexPath.data);
					//std::string texturePath = parentPath.string();
					std::string texturePath = "assets/models/cerberus/textures/Cerberus_R.tga";
					NEO_MESH_LOG("    Roughness map path = {0}", texturePath);
					m_Materials[i]->LoadTexture2D("u_RoughnessTextures", i, texturePath, {});
					materialProperties.HasRoughnessTex = 1.f;
				}
				else
				{
					NEO_MESH_LOG("    No roughness map");
					m_Materials[i]->LoadDefaultTexture2D("u_RoughnessTextures", i);
					materialProperties.Roughness = roughness;
					materialProperties.HasRoughnessTex = 0.f;
				}

				bool metalnessTextureFound = false;
				for (uint32 j = 0; j < aiMaterial->mNumProperties; j++)
				{
					auto prop = aiMaterial->mProperties[j];

					if (true || prop->mType == aiPTI_String)
					{
						//uint32 strLength = *(uint32*)prop->mData;
						//std::string str(prop->mData + 4, strLength);

						std::string key = prop->mKey.data;
						if (true || key == "$raw.ReflectionFactor|file")
						{
							metalnessTextureFound = true;

							std::filesystem::path path = filename;
							auto parentPath = path.parent_path();
							//parentPath /= str;
							//std::string texturePath = parentPath.string();
							std::string texturePath = "assets/models/cerberus/textures/Cerberus_M.tga";
							NEO_MESH_LOG("    Metalness map path = {0}", texturePath);
							m_Materials[i]->LoadTexture2D("u_MetalnessTextures", i, texturePath, {});
							materialProperties.HasMetalnessTex = 1.f;
							break;
						}
					}
				}

				if (!metalnessTextureFound)
				{
					NEO_MESH_LOG("    No metalness map");
					m_Materials[i]->LoadDefaultTexture2D("u_MetalnessTextures", i);
					materialProperties.Metalness = metalness;
					materialProperties.HasMetalnessTex = 0.f;
				}
				NEO_MESH_LOG("------------------------");

				m_Materials[i]->SetProperties(i, &materialProperties);
			}
		}
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::OnUpdate(float deltaSeconds)
	{
		if (m_IsAnimated)
		{
			if (m_AnimationPlaying)
			{
				m_WorldTime += deltaSeconds;

				float ticksPerSecond =
					(float)(m_Scene->mAnimations[0]->mTicksPerSecond != 0 ? m_Scene->mAnimations[0]->mTicksPerSecond : 25.0f) *
					m_TimeMultiplier;
				m_AnimationTime += deltaSeconds * ticksPerSecond;
				m_AnimationTime = fmod(m_AnimationTime, (float)m_Scene->mAnimations[0]->mDuration);
			}

			// TODO: We only need to recalculate bones if rendering has been requested at the current animation frame
			UpdateBoneTransforms(m_AnimationTime);
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

	void Mesh::UpdateBoneTransforms(float time)
	{
		ReadNodeHierarchy(time, m_Scene->mRootNode, glm::mat4(1.0f));

		static std::vector<glm::mat4> boneTransforms;
		boneTransforms.resize(m_BoneInfo.size());
		for (uint32 i = 0; i < m_BoneInfo.size(); i++)
		{
			boneTransforms[i] = m_BoneInfo[i].FinalTransformation;
		}
		m_MeshShader->SetStorageBuffer("BonesUBO", boneTransforms.data());
	}

	void Mesh::ReadNodeHierarchy(float animationTime, const aiNode* pNode, const glm::mat4& parentTransform)
	{
		std::string name(pNode->mName.data);
		const aiAnimation* animation = m_Scene->mAnimations[0];
		glm::mat4 nodeTransform(Mat4FromAssimpMat4(pNode->mTransformation));
		const aiNodeAnim* nodeAnim = FindNodeAnim(animation, name);

		if (nodeAnim)
		{
			glm::vec3 translation = InterpolateTranslation(animationTime, nodeAnim);
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));

			glm::quat rotation = InterpolateRotation(animationTime, nodeAnim);
			glm::mat4 rotationMatrix = glm::toMat4(rotation);

			glm::vec3 scale = InterpolateScale(animationTime, nodeAnim);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));

			nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
		}

		glm::mat4 transform = parentTransform * nodeTransform;

		if (m_BoneMapping.find(name) != m_BoneMapping.end())
		{
			uint32 BoneIndex = m_BoneMapping[name];
			m_BoneInfo[BoneIndex].FinalTransformation = m_InverseTransform * transform * m_BoneInfo[BoneIndex].BoneOffset;
		}

		for (uint32 i = 0; i < pNode->mNumChildren; i++)
		{
			ReadNodeHierarchy(animationTime, pNode->mChildren[i], transform);
		}
	}

	const aiNodeAnim* Mesh::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
	{
		for (uint32 i = 0; i < animation->mNumChannels; i++)
		{
			const aiNodeAnim* nodeAnim = animation->mChannels[i];
			if (std::string(nodeAnim->mNodeName.data) == nodeName)
			{
				return nodeAnim;
			}
		}
		return nullptr;
	}

	uint32 Mesh::FindPosition(float animationTime, const aiNodeAnim* pNodeAnim)
	{
		NEO_CORE_ASSERT(pNodeAnim->mNumPositionKeys > 0);

		for (uint32 i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
		{
			if (animationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
			{
				return i;
			}
		}

		return 0;
	}

	uint32 Mesh::FindRotation(float animationTime, const aiNodeAnim* pNodeAnim)
	{
		NEO_CORE_ASSERT(pNodeAnim->mNumRotationKeys > 0);

		for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
		{
			if (animationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
			{
				return i;
			}
		}

		return 0;
	}

	uint32 Mesh::FindScaling(float animationTime, const aiNodeAnim* pNodeAnim)
	{
		NEO_CORE_ASSERT(pNodeAnim->mNumScalingKeys > 0);

		for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
		{
			if (animationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
			{
				return i;
			}
		}

		return 0;
	}

	glm::vec3 Mesh::InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumPositionKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mPositionKeys[0].mValue;
			return {v.x, v.y, v.z};
		}

		uint32 positionIndex = FindPosition(animationTime, nodeAnim);
		uint32 nextPositionIndex = (positionIndex + 1);
		NEO_CORE_ASSERT(nextPositionIndex < nodeAnim->mNumPositionKeys);
		auto deltaTime = (float)(nodeAnim->mPositionKeys[nextPositionIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime);
		float factor = (animationTime - (float)nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;
		NEO_CORE_ASSERT(factor <= 1.0f, "Factor must be below 1.0f");
		factor = glm::clamp(factor, 0.0f, 1.0f);
		const aiVector3D& start = nodeAnim->mPositionKeys[positionIndex].mValue;
		const aiVector3D& end = nodeAnim->mPositionKeys[nextPositionIndex].mValue;
		aiVector3D delta = end - start;
		auto aiVec = start + factor * delta;
		return {aiVec.x, aiVec.y, aiVec.z};
	}

	glm::quat Mesh::InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumRotationKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mRotationKeys[0].mValue;
			return glm::quat(v.w, v.x, v.y, v.z);
		}

		uint32_t rotationIndex = FindRotation(animationTime, nodeAnim);
		uint32_t nextRotationIndex = (rotationIndex + 1);
		NEO_CORE_ASSERT(nextRotationIndex < nodeAnim->mNumRotationKeys);
		auto deltaTime = (float)(nodeAnim->mRotationKeys[nextRotationIndex].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime);
		float factor = (animationTime - (float)nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;
		NEO_CORE_ASSERT(factor <= 1.0f, "Factor must be below 1.0f");
		factor = glm::clamp(factor, 0.0f, 1.0f);
		const aiQuaternion& startRotationQ = nodeAnim->mRotationKeys[rotationIndex].mValue;
		const aiQuaternion& endRotationQ = nodeAnim->mRotationKeys[nextRotationIndex].mValue;
		auto q = aiQuaternion();
		aiQuaternion::Interpolate(q, startRotationQ, endRotationQ, factor);
		q = q.Normalize();
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	glm::vec3 Mesh::InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumScalingKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mScalingKeys[0].mValue;
			return {v.x, v.y, v.z};
		}

		uint32_t scaleIndex = FindScaling(animationTime, nodeAnim);
		uint32_t nextScaleIndex = (scaleIndex + 1);
		NEO_CORE_ASSERT(nextScaleIndex < nodeAnim->mNumScalingKeys);
		auto deltaTime = (float)(nodeAnim->mScalingKeys[nextScaleIndex].mTime - nodeAnim->mScalingKeys[scaleIndex].mTime);
		float factor = (animationTime - (float)nodeAnim->mScalingKeys[scaleIndex].mTime) / deltaTime;
		NEO_CORE_ASSERT(factor <= 1.0f, "Factor must be below 1.0f");
		factor = glm::clamp(factor, 0.0f, 1.0f);
		const auto& startScale = nodeAnim->mScalingKeys[scaleIndex].mValue;
		const auto& endScale = nodeAnim->mScalingKeys[nextScaleIndex].mValue;
		auto delta = endScale - startScale;
		auto aiVec = startScale + factor * delta;
		return {aiVec.x, aiVec.y, aiVec.z};
	}

} // namespace Neon
