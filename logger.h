#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

class Logger
{
public:
    // Accessor for the singleton instance
    static Logger& Instance();

    // Access the spdlog logger
    spdlog::logger* Get();

    Logger();
    ~Logger();

private:


    // Singleton instance
    static std::unique_ptr<Logger> _instance;

    // Shared pointer to the spdlog logger
    std::shared_ptr<spdlog::logger> _logger;
};
