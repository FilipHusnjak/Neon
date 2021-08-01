#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXContactListener.h"
#include "Neon/Physics/PhysX/PhysXPhysics.h"
#include "Neon/Physics/PhysX/PhysXPhysicsActor.h"
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

	void PhysXPhysicsScene::Destroy()
	{
		PhysicsScene::Destroy();

		NEO_CORE_ASSERT(m_PhysXScene);

		m_PhysXScene->release();
		m_PhysXScene = nullptr;
	}

	SharedRef<PhysicsActor> PhysXPhysicsScene::InternalCreateActor(Entity entity)
	{
		SharedRef<PhysicsActor> actor = SharedRef<PhysXPhysicsActor>::Create(entity);
		NEO_CORE_ASSERT(actor->GetHandle());
		m_PhysXScene->addActor(*static_cast<physx::PxRigidActor*>(actor->GetHandle()));
		return actor;
	}

	void PhysXPhysicsScene::InternalRemoveActor(SharedRef<PhysicsActor> actor)
	{
		NEO_CORE_ASSERT(actor->GetHandle());
		m_PhysXScene->removeActor(*static_cast<physx::PxRigidActor*>(actor->GetHandle()));
	}

} // namespace Neon
