//
// Created by panzhenpeng on 2025/8/10.
//

#ifndef VECTORDB_LOGGER_H
#define VECTORDB_LOGGER_H
#include <memory>
#include <spdlog/spdlog.h>

extern std::shared_ptr<spdlog::logger> GlobalLogger;
void init_global_logger();
void set_log_level(spdlog::level::level_enum log_level);

#endif //VECTORDB_LOGGER_H