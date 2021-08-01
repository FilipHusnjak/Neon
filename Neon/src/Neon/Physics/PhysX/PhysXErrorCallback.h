#pragma once

#include <foundation/PxErrorCallback.h>

#define OVERLAP_MAX_COLLIDERS 10

namespace Neon
{
	class PhysXErrorCallback : public physx::PxErrorCallback
	{
	public:
		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
	};
} // namespace Neon
