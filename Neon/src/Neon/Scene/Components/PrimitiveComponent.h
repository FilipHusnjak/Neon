#pragma once

#include "Neon/Physics/PhysicsBody.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Scene/Components/ActorComponent.h"

namespace Neon
{
	class Actor;

	class PrimitiveComponent : public ActorComponent
	{
	public:
		PrimitiveComponent(Actor* owner);
		PrimitiveComponent(const PrimitiveComponent& other) = default;
		virtual ~PrimitiveComponent();

		virtual void TickComponent(float deltaSeconds) override;

		virtual void CreatePhysicsBody(PhysicsBodyType bodyType, const std::string& boneName = std::string(),
									   const SharedRef<PhysicsMaterial>& material = nullptr);

		virtual SharedRef<PhysicsBody> GetPhysicsBody(const std::string& boneName = std::string()) const
		{
			return m_RootPhysicsBody;
		}

		virtual void AddForce(const glm::vec3& force, const std::string& boneName = std::string());

		virtual void LoadMesh(const std::string& filename) = 0;

		virtual SharedRef<Mesh> GetMesh() const = 0;

		virtual void RenderGui() override;

	protected:
		void RenderMeshProperties(SharedRef<Mesh> mesh) const;

	protected:
		bool m_LockPositionX = false;
		bool m_LockPositionY = false;
		bool m_LockPositionZ = false;
		bool m_LockRotationX = false;
		bool m_LockRotationY = false;
		bool m_LockRotationZ = false;

		SharedRef<PhysicsBody> m_RootPhysicsBody;

	private:
		SharedRef<Texture2D> m_CheckerboardTex;
	};
} // namespace Neon
