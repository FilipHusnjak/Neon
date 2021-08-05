#include "neopch.h"

#include "Neon/Physics/PhysX/PhysXPhysicsDebugger.h"
#include "Neon/Physics/PhysX/PhysXPhysics.h"

namespace Neon
{
	struct PhysXDebuggerData
	{
		physx::PxPvd* Debugger;
		physx::PxPvdTransport* Transport;
	};

	static PhysXDebuggerData* s_DebuggerData = nullptr;

#if NEO_DEBUG

	void PhysXPhysicsDebugger::Initialize()
	{
		s_DebuggerData = new PhysXDebuggerData();

		s_DebuggerData->Debugger = PxCreatePvd(PhysXPhysics::GetFoundation());
		NEO_CORE_ASSERT(s_DebuggerData->Debugger, "PxCreatePvd failed");
	}

	void PhysXPhysicsDebugger::Shutdown()
	{
		s_DebuggerData->Debugger->release();
		delete s_DebuggerData;
		s_DebuggerData = nullptr;
	}

	void PhysXPhysicsDebugger::StartDebugging()
	{
		StopDebugging();

		s_DebuggerData->Transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 1000);
		s_DebuggerData->Debugger->connect(*s_DebuggerData->Transport, physx::PxPvdInstrumentationFlag::eALL);
	}

	bool PhysXPhysicsDebugger::IsDebugging()
	{
		return s_DebuggerData->Debugger->isConnected();
	}

	void PhysXPhysicsDebugger::StopDebugging()
	{
		if (!s_DebuggerData->Debugger->isConnected())
			return;

		s_DebuggerData->Debugger->disconnect();
		s_DebuggerData->Transport->release();
	}

	physx::PxPvd* PhysXPhysicsDebugger::GetDebugger()
	{
		return s_DebuggerData->Debugger;
	}

#else

	void PhysXPhysicsDebugger::Initialize()
	{
	}
	void PhysXPhysicsDebugger::Shutdown()
	{
	}
	void PhysXPhysicsDebugger::StartDebugging()
	{
	}
	bool PhysXPhysicsDebugger::IsDebugging()
	{
		return false;
	}
	void PhysXPhysicsDebugger::StopDebugging()
	{
	}
	physx::PxPvd* PhysXPhysicsDebugger::GetDebugger()
	{
		return nullptr;
	}

#endif
}
