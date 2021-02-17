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

namespace Assimp
{
	class Importer;
}

namespace Neon
{
#define MAX_BONES_PER_VERTEX 10

	struct StaticVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		uint32 MaterialIndex;
		glm::vec2 Texcoord;
	};

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		uint32 MaterialIndex;
		glm::vec2 Texcoord;

		uint32 BoneIds[MAX_BONES_PER_VERTEX] = {};
		float Weights[MAX_BONES_PER_VERTEX] = {};

		void AddBoneData(uint32 boneId, float weight)
		{
			for (uint32 i = 0; i < MAX_BONES_PER_VERTEX; i++)
			{
				if (Weights[i] == 0.0)
				{
					BoneIds[i] = boneId;
					Weights[i] = weight;
					return;
				}
			}

			// TODO: Keep top weights
			NEO_CORE_WARN(
				"Vertex has more than {0} bones/weights affecting it, extra data will be discarded (BoneID={1}, Weight={2})",
				MAX_BONES_PER_VERTEX, boneId, weight);
		}
	};

	struct Index
	{
		uint32 V1, V2, V3;
	};

	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};

	struct VertexBoneData
	{
		uint32 Ids[10];
		float Weights[10];

		VertexBoneData()
		{
			memset(Ids, 0, sizeof(Ids));
			memset(Weights, 0, sizeof(Weights));
		};

		void AddBoneData(uint32_t BoneID, float Weight)
		{
			for (size_t i = 0; i < MAX_BONES_PER_VERTEX; i++)
			{
				if (Weights[i] == 0.0)
				{
					Ids[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}

			NEO_CORE_ASSERT(false, "Too many bones!");
		}
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

		void OnUpdate(float deltaSeconds);

		const SharedRef<GraphicsPipeline>& GetGraphicsPipeline() const
		{
			return m_MeshGraphicsPipeline;
		}

		const SharedRef<Shader>& GetShader() const
		{
			return m_MeshShader;
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

		const std::vector<Material>& GetMaterials() const
		{
			return m_Materials;
		}

		const std::string& GetFilePath() const
		{
			return m_FilePath;
		}

	private:
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32 level = 0);
		void UpdateBoneTransforms(float time);
		void ReadNodeHierarchy(float animationTime, const aiNode* pNode, const glm::mat4& parentTransform);

		const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
		uint32 FindPosition(float animationTime, const aiNodeAnim* pNodeAnim);
		uint32 FindRotation(float animationTime, const aiNodeAnim* pNodeAnim);
		uint32 FindScaling(float animationTime, const aiNodeAnim* pNodeAnim);
		glm::vec3 InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::quat InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim);

	private:
		std::vector<Submesh> m_Submeshes;

		UniqueRef<Assimp::Importer> m_Importer;

		glm::mat4 m_InverseTransform;

		std::vector<BoneInfo> m_BoneInfo;

		SharedRef<VertexBuffer> m_VertexBuffer;
		SharedRef<IndexBuffer> m_IndexBuffer;

		std::vector<StaticVertex> m_StaticVertices;
		std::vector<AnimatedVertex> m_AnimatedVertices;
		std::vector<Index> m_Indices;
		std::unordered_map<std::string, uint32> m_BoneMapping;
		std::vector<glm::mat4> m_BoneTransforms;

		const aiScene* m_Scene;

		SharedRef<Shader> m_MeshShader;
		SharedRef<GraphicsPipeline> m_MeshGraphicsPipeline;

		std::vector<Material> m_Materials;

		// Animation
		bool m_IsAnimated = false;
		float m_AnimationTime = 0.0f;
		float m_WorldTime = 0.0f;
		float m_TimeMultiplier = 1.0f;
		bool m_AnimationPlaying = true;

		std::string m_FilePath;
	};
} // namespace Neon
