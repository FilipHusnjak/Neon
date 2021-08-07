#pragma once

#include "Neon/Renderer/Mesh.h"

namespace Neon
{
#define MAX_BONES_PER_VERTEX 10

	class SkeletalMesh : public Mesh
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

		struct BoneInfo
		{
			std::string Name;
			int32 ParentBoneIndex = -1;
			glm::mat4 BoneTransform;
			glm::mat4 NodeTransform;
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

	public:
		SkeletalMesh(const std::string& filename);
		virtual ~SkeletalMesh() = default;

		void TickAnimation(float deltaSeconds);

		BoneInfo& GetBoneInfo(const std::string& boneName = std::string());

	private:
		std::vector<BoneInfo> m_Skeleton;
		std::vector<Vertex> m_Vertices;

		std::unordered_map<std::string, uint32> m_BoneMapping;

		// Animation
		bool m_IsAnimated = false;
		float m_AnimationTime = 0.0f;
		float m_WorldTime = 0.0f;
		float m_TimeMultiplier = 1.0f;
		bool m_AnimationPlaying = true;

	private:
		void UpdateBoneTransforms();
		void ReadNodeHierarchy(float animationTime, const aiNode* pNode, const glm::mat4& parentTransform);

		const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
		uint32 FindPosition(float animationTime, const aiNodeAnim* pNodeAnim);
		uint32 FindRotation(float animationTime, const aiNodeAnim* pNodeAnim);
		uint32 FindScaling(float animationTime, const aiNodeAnim* pNodeAnim);
		glm::vec3 InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::quat InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim);
	};
} // namespace Neon
