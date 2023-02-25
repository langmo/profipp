#ifndef LOGGING_H
#define LOGGING_H
#pragma once

#include <functional>
#include <string>
#include <iostream>

namespace profinet
{
enum LogLevel
{
    logFatal = 10,
    logError = 20,
    logWarning = 30,
    logInfo = 40,
    logDebug = 50
};
using LoggerType = std::function<void(LogLevel logLevel, const std::string& message)>;

namespace logging
{
template<LogLevel maxLogLevel=logInfo> LoggerType CreateConsoleLogger()
{
    return [](LogLevel logLevel, const std::string& message)
    {
        if(logLevel <= maxLogLevel)
            std::cout << message << std::endl;
    };
}
}
}
#endif