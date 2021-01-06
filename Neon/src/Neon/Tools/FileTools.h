#pragma once

#include <fstream>
#include <vector>

namespace Neon
{
std::vector<char> ReadFile(const std::string& filename);
} // namespace Neon