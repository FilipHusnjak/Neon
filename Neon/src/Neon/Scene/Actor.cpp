#include "neopch.h"

#include "Neon/Scene/Actor.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Neon
{
	Actor::Actor(entt::entity handle, Scene* scene, const std::string& tag, UUID id, glm::vec3 translation /*= glm::vec3()*/,
				 glm::vec3 rotation /*= glm::vec3()*/, glm::vec3 scale /*= glm::vec3(1.f)*/)
		: m_EntityHandle(handle)
		, m_Scene(scene)
		, m_Tag(tag)
		, m_ID(id)
		, m_Translation(translation)
		, m_Rotation(rotation)
		, m_Scale(scale)
	{

	}

	const glm::mat4 Actor::GetTransformMat()
	{
		return glm::translate(glm::mat4(1.f), m_Translation) * glm::eulerAngleXYZ(m_Rotation.x, m_Rotation.y, m_Rotation.z) *
			   glm::scale(glm::mat4(1.f), glm::abs(m_Scale));
	}

	void Actor::Tick(float deltaSeconds)
	{
		for (ActorComponent* component : m_ActorComponents)
		{
			NEO_CORE_ASSERT(component);
			component->TickComponent(deltaSeconds);
		}
	}

} // namespace Neon
