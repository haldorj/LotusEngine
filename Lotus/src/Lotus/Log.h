#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Lotus {

	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core log macros
#define LOTUS_CORE_ERROR(...)  ::Lotus::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOTUS_CORE_WARN(...)   ::Lotus::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOTUS_CORE_INFO(...)   ::Lotus::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOTUS_CORE_TRACE(...)  ::Lotus::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LOTUS_CORE_FATAL(...)  ::Lotus::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define LOTUS_ERROR(...)  ::Lotus::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOTUS_WARN(...)   ::Lotus::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOTUS_INFO(...)   ::Lotus::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOTUS_TRACE(...)  ::Lotus::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LOTUS_FATAL(...)  ::Lotus::Log::GetClientLogger()->fatal(__VA_ARGS__)