#pragma once

#include <fstream>
#include <vector>

namespace Neon
{
	class File
	{
	public:
		template <typename T>
		static bool ReadFile(const std::string& filename, std::vector<T>& data, bool binary)
		{
			std::ios::openmode flags = std::ios::ate;
			if (binary)
			{
				flags |= std::ios::binary;
			}
			std::ifstream file(filename, flags);

			if (!file.is_open())
			{
				return false;
			}

			size_t fileSize = (size_t)file.tellg();
			data.resize(fileSize / sizeof(T));
			file.seekg(0);
			file.read(reinterpret_cast<char*>(data.data()), fileSize);
			file.close();

			return true;
		}

		template <typename T>
		static void WriteToFile(const std::string& filename, const std::vector<T> data, bool binary)
		{
			std::ios::openmode flags = std::ios::app;
			if (binary)
			{
				flags |= std::ios::binary;
			}
			std::ofstream file;
			file.open(filename, flags);

			if (!file.is_open())
			{
				file.open(filename, flags);
			}
		//	NEO_CORE_ASSERT(file.is_open(), "Could not create or open file!");

			file.write(reinterpret_cast<const char*>(data.data()), sizeof(T));
			file.close();
		}
	};

} // namespace Neon