#include "neopch.h"

#include "PhysXErrorCallback.h"

namespace Neon
{
	void PhysXErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		NEO_CORE_ERROR(message);
	}
} // namespace Neon
