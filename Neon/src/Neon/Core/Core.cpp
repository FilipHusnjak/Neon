#include "neopch.h"

#include "Core.h"

namespace Neon
{
	void InitializeCore()
	{
		Neon::Log::Init();

		NEO_CORE_TRACE("Neon Engine");
		NEO_CORE_TRACE("Initializing...");
	}

	void ShutdownCore()
	{
		NEO_CORE_TRACE("Shutting down...");
	}
} // namespace Neon
