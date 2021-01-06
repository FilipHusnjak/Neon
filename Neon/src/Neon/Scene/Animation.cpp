#include "neopch.h"

#include "Animation.h"
#include <glm/gtc/matrix_transform.hpp>

Neon::Animation::Animation(const aiScene* scene, int index,
						   std::unordered_map<std::string, uint32_t>& boneMap, uint32_t bonesCount)
{
	assert(scene->HasAnimations());
	assert(index < scene->mNumAnimations);
	m_Duration = static_cast<float>(scene->mAnimations[index]->mDuration);
	m_TicksPerSecond = scene->mAnimations[index]->mTicksPerSecond != 0
						   ? static_cast<float>(scene->mAnimations[0]->mTicksPerSecond)
						   : 25.0f;
	m_ScalingKeyFrames.resize(bonesCount);
	m_PositionKeyFrames.resize(bonesCount);
	m_RotationKeyFrames.resize(bonesCount);
	LoadAnimation(scene, scene->mRootNode, index, boneMap);
}

void Neon::Animation::LoadAnimation(const aiScene* scene, const aiNode* node, int animationIndex,
									std::unordered_map<std::string, uint32_t>& boneMap)
{
	std::string nodeName = node->mName.data;

	const aiAnimation* animation = scene->mAnimations[animationIndex];

	const aiNodeAnim* nodeAnim = nullptr;
	for (int i = 0; i < animation->mNumChannels; i++)
	{
		const aiNodeAnim* nodeAnimTemp = animation->mChannels[i];
		if (std::string(nodeAnimTemp->mNodeName.data) == nodeName)
		{
			nodeAnim = nodeAnimTemp;
			break;
		}
	}

	if (nodeAnim)
	{
		assert(boneMap.find(nodeName) != boneMap.end());
		uint32_t id = boneMap[nodeName];
		for (int i = 0; i < nodeAnim->mNumScalingKeys; i++)
		{
			const auto& scalingKey = nodeAnim->mScalingKeys[i];
			m_ScalingKeyFrames[id].push_back(
				{static_cast<float>(scalingKey.mTime), *(glm::vec3*)&scalingKey.mValue});
		}
		for (int i = 0; i < nodeAnim->mNumPositionKeys; i++)
		{
			const auto& positionKey = nodeAnim->mPositionKeys[i];
			m_PositionKeyFrames[id].push_back(
				{static_cast<float>(positionKey.mTime), *(glm::vec3*)&positionKey.mValue});
		}
		for (int i = 0; i < nodeAnim->mNumRotationKeys; i++)
		{
			const auto& rotationKey = nodeAnim->mRotationKeys[i];
			m_RotationKeyFrames[id].push_back(
				{static_cast<float>(rotationKey.mTime), rotationKey.mValue});
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		LoadAnimation(scene, node->mChildren[i], animationIndex, boneMap);
	}
}

void Neon::Animation::Update(float seconds, std::vector<glm::mat4>& transforms, Bone& rootBone)
{
	CalculateBoneTransforms(rootBone, glm::mat4(1.0f), transforms);
	m_CurrentAnimationTime += seconds * m_TicksPerSecond;
	m_CurrentAnimationTime = fmod(m_CurrentAnimationTime, m_Duration);
}

void Neon::Animation::Reset()
{
	m_CurrentAnimationTime = 0;
}

void Neon::Animation::CalculateBoneTransforms(Bone& bone, glm::mat4 parentTransform,
											  std::vector<glm::mat4>& transforms)
{
	glm::mat4 nodeTransform = bone.GetLocalTransform();
	if (bone.m_Animated)
	{
		glm::mat4 scaling = glm::scale(
			glm::mat4(1.0), Interpolate<KeyFrameVector, glm::vec3>(
								m_CurrentAnimationTime, m_ScalingKeyFrames[bone.GetID()]));
		glm::mat4 translation = glm::translate(
			glm::mat4(1.0), Interpolate<KeyFrameVector, glm::vec3>(
								m_CurrentAnimationTime, m_PositionKeyFrames[bone.GetID()]));

		auto mat = Interpolate<KeyFrameQuaternion, aiQuaternion>(m_CurrentAnimationTime,
																 m_RotationKeyFrames[bone.GetID()])
					   .GetMatrix();
		auto rotation = glm::mat4(glm::transpose(*(glm::mat3*)&mat));
		nodeTransform = translation * rotation * scaling;
	}

	auto newParentTransform = parentTransform * bone.GetParentTransform() * nodeTransform;
	transforms[bone.GetID()] = newParentTransform * bone.GetOffsetMatrix();
	bone.SetAnimatedTransform(transforms[bone.GetID()]);

	for (auto& child : bone.GetChildren())
	{
		CalculateBoneTransforms(child, newParentTransform, transforms);
	}
}
