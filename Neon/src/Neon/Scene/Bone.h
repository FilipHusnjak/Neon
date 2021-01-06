//
// Created by Filip on 17.8.2020..
//

#ifndef NEON_BONE_H
#define NEON_BONE_H

#include <glm/glm.hpp>

namespace Neon
{
class Bone
{
public:
	Bone()
		: m_ID(-1)
	{
	}
	Bone(uint32_t ID, glm::mat4 offsetMatrix, glm::mat4 parentTransform, glm::mat4 localTransform)
		: m_ID(ID)
		, m_OffsetMatrix(offsetMatrix)
		, m_ParentTransform(parentTransform)
		, m_LocalTransform(localTransform)
	{
	}
	bool m_Animated = true;
	[[nodiscard]] uint32_t GetID() const
	{
		return m_ID;
	}
	[[nodiscard]] glm::mat4 GetOffsetMatrix() const
	{
		return m_OffsetMatrix;
	}
	[[nodiscard]] glm::mat4 GetParentTransform() const
	{
		return m_ParentTransform;
	}
	[[nodiscard]] glm::mat4 GetLocalTransform() const
	{
		return m_LocalTransform;
	}
	void SetAnimatedTransform(glm::mat4 animatedTransform)
	{
		m_AnimatedTransform = animatedTransform;
	}
	[[nodiscard]] glm::mat4 GetAnimatedTransform() const
	{
		return m_AnimatedTransform;
	}
	std::vector<Bone>& GetChildren()
	{
		return m_Children;
	}

private:
	uint32_t m_ID;
	glm::mat4 m_OffsetMatrix{};
	glm::mat4 m_ParentTransform{};
	glm::mat4 m_LocalTransform{};
	glm::mat4 m_AnimatedTransform{};
	std::vector<Bone> m_Children;
};
} // namespace Neon

#endif //NEON_BONE_H
