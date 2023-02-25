#ifndef ABSTRACTLOGGER_H
#define ABSTRACTLOGGER_H

#pragma once

#include <memory>
namespace profinet
{
class Logger
{
public:
    Logger() = default;
    virtual ~Logger() = default;
    Logger (const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;
    virtual void PrintDebug(const char* entry, ...) = 0;
    virtual void PrintInfo(const char* entry, ...) = 0;
    virtual void PrintWarning(const char* entry, ...) = 0;
    virtual void PrintError(const char* entry, ...) = 0;
    virtual void PrintFatal(const char* entry, ...) = 0;
};
using PLogger = std::shared_ptr<Logger>;
}
#endif