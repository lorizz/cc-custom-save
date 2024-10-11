#include "logger.h"
#include <iostream>
#include <memory>

std::unique_ptr<Logger> Logger::_instance = nullptr;

Logger& Logger::Instance()
{
    // Create the instance if it does not exist
    if (!_instance)
    {
        _instance = std::make_unique<Logger>();
    }
    return *_instance;
}

Logger::Logger()
{
    try
    {
        _logger = spdlog::basic_logger_st("CustomSave", "../CustomSave.log");
    }
    catch (const spdlog::spdlog_ex& exception)
    {
        std::cerr << "Logger initialization failed: " << exception.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

spdlog::logger* Logger::Get()
{
    return _logger.get();
}

Logger::~Logger()
{
    // No explicit cleanup needed for spdlog, it handles its own destruction
}
