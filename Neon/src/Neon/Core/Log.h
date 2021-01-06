#pragma once

#include <spdlog/spdlog.h>

namespace Neon
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
		{
			return s_CoreLogger;
		}
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger()
		{
			return s_ClientLogger;
		}

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
} // namespace Neon

// Core Logging Macros
#define NEO_CORE_TRACE(...) Neon::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define NEO_CORE_INFO(...) Neon::Log::GetCoreLogger()->info(__VA_ARGS__)
#define NEO_CORE_WARN(...) Neon::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define NEO_CORE_ERROR(...) Neon::Log::GetCoreLogger()->error(__VA_ARGS__)
#define NEO_CORE_FATAL(...) Neon::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client Logging Macros
#define NEO_TRACE(...) Neon::Log::GetClientLogger()->trace(__VA_ARGS__)
#define NEO_INFO(...) Neon::Log::GetClientLogger()->info(__VA_ARGS__)
#define NEO_WARN(...) Neon::Log::GetClientLogger()->warn(__VA_ARGS__)
#define NEO_ERROR(...) Neon::Log::GetClientLogger()->error(__VA_ARGS__)
#define NEO_FATAL(...) Neon::Log::GetClientLogger()->critical(__VA_ARGS__)
