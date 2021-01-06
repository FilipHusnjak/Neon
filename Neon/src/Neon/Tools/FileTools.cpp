#include "neopch.h"

#include "FileTools.h"

std::vector<char> Neon::ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	NEO_CORE_ASSERT(file.is_open(), "Could not open file " + filename);

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}
