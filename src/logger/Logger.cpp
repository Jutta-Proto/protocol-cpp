#include "logger/Logger.hpp"

#include <memory>
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <iso646.h>
#endif  // _WIN32

namespace logger {
constexpr int THREAD_QUEUE_LENGTH = 8192;
constexpr int FILE_ROTATION_TIME = 1048576 * 5;

void setup_logger(const spdlog::level::level_enum level) {
    if (not std::filesystem::exists(logger::log_folder)) {
        std::filesystem::create_directory(logger::log_folder);
    }
    spdlog::init_thread_pool(THREAD_QUEUE_LENGTH, 1);
    spdlog::sink_ptr console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%^%=8l%$] [thread %t]\t%v");
#ifdef _WIN32
    std::string s = (logger::log_folder / "jutta.log").string();
    spdlog::sink_ptr file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(s, FILE_ROTATION_TIME, 3);
#else  // _WIN32
    spdlog::sink_ptr file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logger::log_folder / "jutta.log", FILE_ROTATION_TIME, 3);
#endif
    file_sink->set_pattern("[%H:%M:%S %z] [%=8l] [thread %t] [%@]\t%v");
    std::vector<spdlog::sink_ptr> sinks{file_sink, console_sink};
    std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::async_logger>("", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(level);
    spdlog::set_default_logger(logger);
}

void set_log_level(const spdlog::level::level_enum level) {
    spdlog::default_logger()->set_level(level);
}

void deactivate_logger() {
    logger::set_log_level(spdlog::level::off);
}
}  // namespace logger
