#pragma once

#include <memory>

#include "core.h"
#include "spdlog/spdlog.h"

namespace MH
{
class Log
{
public:
    static void init();
    
    inline static std::shared_ptr<spdlog::logger>& get_core_logger() { return s_core_logger; }
    inline static std::shared_ptr<spdlog::logger>& get_client_logger() { return s_client_logger; }

    static std::shared_ptr<spdlog::logger> s_client_logger;
    static std::shared_ptr<spdlog::logger> s_core_logger;
};

#define CORE_LOG_ERROR(...) MH::Log::get_core_logger()->error(__VA_ARGS__)
#define CORE_LOG_WARN(...) MH::Log::get_core_logger()->warn(__VA_ARGS__)
#define CORE_LOG_INFO(...) MH::Log::get_core_logger()->info(__VA_ARGS__)
#define CORE_LOG_TRACE(...) MH::Log::get_core_logger()->trace(__VA_ARGS__)
#define CORE_LOG_FATAL(...) MH::Log::get_core_logger()->fatal(__VA_ARGS__)

#define LOG_ERROR(...) MH::Log::get_client_logger()->error(__VA_ARGS__)
#define LOG_WARN(...) MH::Log::get_client_logger()->warn(__VA_ARGS__)
#define LOG_INFO(...) MH::Log::get_client_logger()->info(__VA_ARGS__)
#define LOG_TRACE(...) MH::Log::get_client_logger()->trace(__VA_ARGS__)
#define LOG_FATAL(...) MH::Log::get_client_logger()->fatal(__VA_ARGS__)
} // namespace MH
