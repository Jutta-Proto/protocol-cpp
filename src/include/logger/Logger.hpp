#pragma once

#include <filesystem>
#include <spdlog/spdlog.h>

namespace logger {
const std::filesystem::path log_folder("logs");
// Setup the logger, note the loglevel can not be set below the CMAKE log level (To change this use -DLOG_LEVEL=...)
void setup_logger(const spdlog::level::level_enum level);
void set_log_level(const spdlog::level::level_enum level);
void deactivate_logger();
}  // namespace logger
