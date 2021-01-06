//
// Created by Filip on 17.8.2020..
//

#ifndef NEON_ANIMATION_H
#define NEON_ANIMATION_H

#include "Bone.h"
#include <assimp/scene.h>

namespace Neon
{
struct KeyFrameVector
{
	float time{};
	glm::vec3 value{};

	[[nodiscard]] glm::vec3 Interpolate(glm::vec3 nextValue, float factor) const
	{
		return factor * nextValue + (1.0f - factor) * value;
	}
};

struct KeyFrameQuaternion
{
	float time{};
	// TODO: write own quaternion class
	aiQuaternion value{};

	[[nodiscard]] aiQuaternion Interpolate(aiQuaternion nextValue, float factor) const
	{
		aiQuaternion result;
		aiQuaternion::Interpolate(result, value, nextValue, factor);
		return result;
	}
};

class Animation
{
public:
	Animation(const aiScene* scene, int index, std::unordered_map<std::string, uint32_t>& boneMap,
			  uint32_t bonesCount);
	void Update(float seconds, std::vector<glm::mat4>& transforms, Bone& rootBone);
	void Reset();

private:
	void LoadAnimation(const aiScene* scene, const aiNode* node, int animationIndex,
					   std::unordered_map<std::string, uint32_t>& boneMap);
	void CalculateBoneTransforms(Bone& bone, glm::mat4 parentTransform,
								 std::vector<glm::mat4>& transforms);

private:
	std::vector<std::vector<KeyFrameVector>> m_ScalingKeyFrames;
	std::vector<std::vector<KeyFrameVector>> m_PositionKeyFrames;
	std::vector<std::vector<KeyFrameQuaternion>> m_RotationKeyFrames;

	float m_Duration;
	float m_TicksPerSecond;
	float m_CurrentAnimationTime = 0;

	template<typename T>
	int FindIndex(float animationTime, const std::vector<T>& keyFrames)
	{
		assert(keyFrames.size() > 1);
		int i = 0;
		for (const auto& keyFrame : keyFrames)
		{
			if (animationTime < keyFrame.time) { return i - 1; }
			i++;
		}
		return -1;
	}

	template<typename T, typename V>
	V Interpolate(float animationTime, const std::vector<T>& keyFrames)
	{
		assert(!keyFrames.empty());
		if (keyFrames.size() == 1) { return {keyFrames[0].value}; }
		int firstIndex = FindIndex(animationTime, keyFrames);
		assert(firstIndex >= 0);
		int nextIndex = firstIndex + 1;
		assert(nextIndex < keyFrames.size());
		auto deltaTime = static_cast<float>(keyFrames[nextIndex].time - keyFrames[firstIndex].time);
		float factor = (animationTime - (float)keyFrames[firstIndex].time) / deltaTime;
		assert(factor >= 0.0f && factor <= 1.0f);
		return keyFrames[firstIndex].Interpolate(keyFrames[nextIndex].value, factor);
	}
};
} // namespace Neon

#endif //NEON_ANIMATION_H
