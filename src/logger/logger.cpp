//
// Created by panzhenpeng on 2025/8/10.
//
#include <logger/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> GlobalLogger;

void init_global_logger()
{
    GlobalLogger = spdlog::stdout_color_mt("GlobalLogger");
}

void set_log_level(spdlog::level::level_enum log_level)
{
    GlobalLogger->set_level(log_level);
}