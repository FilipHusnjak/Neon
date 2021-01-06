#pragma once

#include <memory>

using uint8 = uint8_t;
using int8 = int8_t;

using uint16 = uint16_t;
using int16 = int16_t;

using uint32 = uint32_t;
using int32 = int32_t;

using uint64 = uint64_t;
using int64 = int64_t;

// __VA_ARGS__ expansion to get past MSVC "bug"
#define NEO_EXPAND_VARGS(x) x

#define BIT(x) (1 << x)

#include "Assert.h"
#include "Log.h"
#include "SharedRef.h"

#ifndef NEO_PLATFORM_WINDOWS
	#error Only Windows are supported!
#endif

namespace Neon
{
	void InitializeCore();
	void ShutdownCore();

	template<typename T>
	using UniqueRef = std::unique_ptr<T>;
	template<typename T, typename... Args>
	constexpr UniqueRef<T> CreateUnique(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
} // namespace Neon
