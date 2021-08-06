#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXContactListener.h"
#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsBody.h"
#include "Neon/Physics/PhysX/PhysXPhysicsScene.h"

namespace Neon
{
	static PhysXContactListener s_ContactListener;

	PhysXPhysicsScene::PhysXPhysicsScene(const PhysicsSettings& settings)
		: PhysicsScene(settings)
	{
		physx::PxPhysics* physXSDK = static_cast<physx::PxPhysics*>(Physics::GetPhysicsSDK());
		NEO_CORE_ASSERT(physXSDK);

		const SharedRef<PhysXPhysics> physXPhysics = Physics::Get().As<PhysXPhysics>();
		NEO_CORE_ASSERT(physXPhysics);

		physx::PxSceneDesc sceneDesc(physXSDK->getTolerancesScale());
		sceneDesc.flags |=
			physx::PxSceneFlag::eENABLE_CCD |
			physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS; // TODO: | physx::PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS
		sceneDesc.gravity = PhysXUtils::ToPhysXVector(settings.Gravity);
		sceneDesc.broadPhaseType = PhysXUtils::NeonToPhysXBroadphaseType(settings.BroadphaseAlgorithm);
		sceneDesc.cpuDispatcher = physXPhysics->GetCPUDispatcher();
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		sceneDesc.simulationEventCallback = &s_ContactListener;
		sceneDesc.frictionType = PhysXUtils::NeonToPhysXFrictionType(settings.FrictionModel);

		NEO_CORE_ASSERT(sceneDesc.isValid());

		m_PhysXScene = physXSDK->createScene(sceneDesc);
		NEO_CORE_ASSERT(m_PhysXScene);
	}

	PhysXPhysicsScene::~PhysXPhysicsScene()
	{
	}

	void PhysXPhysicsScene::Tick(float deltaSeconds)
	{
		uint32 substepCount = GetNumSubsteps(deltaSeconds);

		if (substepCount <= 0)
		{
			return;			
		}

		for (uint32_t i = 0; i < substepCount; i++)
		{
			m_PhysXScene->simulate(m_SubStepSize);
			m_PhysXScene->fetchResults(true);
		}
	}

	void PhysXPhysicsScene::Destroy()
	{
		for (auto& physicsBody : m_PhysicsBodies)
		{
			m_PhysXScene->removeActor(*static_cast<physx::PxRigidActor*>(physicsBody->GetHandle()));
		}

		NEO_CORE_ASSERT(m_PhysXScene);

		m_PhysXScene->release();
		m_PhysXScene = nullptr;

		PhysicsScene::Destroy();
	}

	SharedRef<PhysicsConstraint> PhysXPhysicsScene::AddPhysicsConstraint(const SharedRef<PhysicsBody>& body0,
																					 const SharedRef<PhysicsBody>& body1)
	{
		return PhysicsScene::AddPhysicsConstraint(body0, body1);
	}

	void PhysXPhysicsScene::RemovePhysicsConstraint(SharedRef<PhysicsConstraint>& physicsConstraint)
	{
		NEO_CORE_ASSERT(physicsConstraint);
		NEO_CORE_ASSERT(physicsConstraint->GetHandle());
		PhysicsScene::RemovePhysicsConstraint(physicsConstraint);
	}

	SharedRef<PhysicsBody> PhysXPhysicsScene::AddPhysicsBody(PhysicsBodyType physicsBodyType, const Transform& transform)
	{
		SharedRef<PhysicsBody> physicsBody = PhysicsScene::AddPhysicsBody(physicsBodyType, transform);

		NEO_CORE_ASSERT(physicsBody);
		NEO_CORE_ASSERT(physicsBody->GetHandle());
		m_PhysXScene->addActor(*static_cast<physx::PxRigidActor*>(physicsBody->GetHandle()));
		return physicsBody;
	}

	void PhysXPhysicsScene::RemovePhysicsBody(SharedRef<PhysicsBody>& physicsBody)
	{
		NEO_CORE_ASSERT(physicsBody);
		NEO_CORE_ASSERT(physicsBody->GetHandle());
		m_PhysXScene->removeActor(*static_cast<physx::PxRigidActor*>(physicsBody->GetHandle()));

		PhysicsScene::RemovePhysicsBody(physicsBody);
	}

} // namespace Neon
