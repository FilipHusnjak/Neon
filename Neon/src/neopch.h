#pragma once

// Needed for windows.h header
#ifndef NOMINMAX
	#define NOMINMAX
#endif

#ifdef NEO_PLATFORM_WINDOWS
	#include <windows.h>
#endif

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>

#include <array>
#include <sstream>
#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <queue>

#include "Neon/Core/Core.h"

#include <Windows.h>
