#include "neopch.h"

#include "Neon/Renderer/SceneRenderer.h"
#include "Neon/Renderer/SkeletalMesh.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Neon
{
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

	SkeletalMesh::SkeletalMesh(const std::string& filename)
		: Mesh(filename)
	{
		NEO_CORE_ASSERT(m_Scene);

		m_IsAnimated = m_Scene->mAnimations != nullptr;

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
		shaderSpecification.ShaderPaths[ShaderType::Vertex] = "assets/shaders/PbrSkeletal_Vert.glsl";
		shaderSpecification.ShaderVariableCounts["MaterialUBO"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_AlbedoTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_NormalTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_RoughnessTextures"] = m_Scene->mNumMaterials;
		shaderSpecification.ShaderVariableCounts["u_MetalnessTextures"] = m_Scene->mNumMaterials;

		// Bones
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
					m_BoneInfo[boneIndex].FinalTransformation = glm::mat4(1.f);
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
					m_Vertices[vertexId].AddBoneData(boneIndex, weight);
				}
			}
		}

		std::vector<VertexBufferElement> elements = {{ShaderDataType::Float3}, {ShaderDataType::Float3}, {ShaderDataType::Float3},
													 {ShaderDataType::Float3}, {ShaderDataType::UInt},	 {ShaderDataType::Float2}};
		for (uint32 i = 0; i < MAX_BONES_PER_VERTEX; i++)
		{
			elements.emplace_back(ShaderDataType::UInt);
		}
		for (uint32 i = 0; i < MAX_BONES_PER_VERTEX; i++)
		{
			elements.emplace_back(ShaderDataType::Float);
		}

		VertexBufferLayout vertexBufferLayout = elements;
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), static_cast<uint32>(m_Vertices.size()) * sizeof(Vertex),
											  vertexBufferLayout);

		shaderSpecification.VBLayout = vertexBufferLayout;
		shaderSpecification.ShaderVariableCounts["BonesUBO"] = static_cast<uint32>(m_BoneInfo.size());

		ShaderSpecification wireframeShaderSpecification;
		wireframeShaderSpecification.ShaderPaths[ShaderType::Fragment] = "assets/shaders/Wireframe_Frag.glsl";
		wireframeShaderSpecification.ShaderPaths[ShaderType::Vertex] = "assets/shaders/WireframeAnim_Vert.glsl";
		wireframeShaderSpecification.VBLayout = vertexBufferLayout;
		wireframeShaderSpecification.ShaderVariableCounts["BonesUBO"] = static_cast<uint32>(m_BoneInfo.size());

		CreateShaderAndGraphicsPipeline(shaderSpecification, wireframeShaderSpecification);

		UpdateBoneTransforms();
	}

	void SkeletalMesh::TickAnimation(float deltaSeconds)
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
			ReadNodeHierarchy(m_AnimationTime, m_Scene->mRootNode, glm::mat4(1.0f));
			UpdateBoneTransforms();
		}
	}

	void SkeletalMesh::UpdateBoneTransforms()
	{
		static std::vector<glm::mat4> boneTransforms;
		boneTransforms.resize(m_BoneInfo.size());
		for (uint32 i = 0; i < m_BoneInfo.size(); i++)
		{
			boneTransforms[i] = m_BoneInfo[i].FinalTransformation;
		}
		m_MeshShader->SetStorageBuffer("BonesUBO", boneTransforms.data());
		m_WireframeMeshShader->SetStorageBuffer("BonesUBO", boneTransforms.data());
	}

	void SkeletalMesh::ReadNodeHierarchy(float animationTime, const aiNode* pNode, const glm::mat4& parentTransform)
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

	const aiNodeAnim* SkeletalMesh::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
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

	uint32 SkeletalMesh::FindPosition(float animationTime, const aiNodeAnim* pNodeAnim)
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

	uint32 SkeletalMesh::FindRotation(float animationTime, const aiNodeAnim* pNodeAnim)
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

	uint32 SkeletalMesh::FindScaling(float animationTime, const aiNodeAnim* pNodeAnim)
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

	glm::vec3 SkeletalMesh::InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim)
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

	glm::quat SkeletalMesh::InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim)
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

	glm::vec3 SkeletalMesh::InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim)
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
