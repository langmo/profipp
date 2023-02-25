#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#pragma once

#include "Logger.h"
namespace profinet
{
class ConsoleLogger : public Logger
{
public:
    ConsoleLogger();
    
    ConsoleLogger (const ConsoleLogger&) = delete;
    ConsoleLogger& operator= (const ConsoleLogger&) = delete;

    virtual ~ConsoleLogger();
    virtual void PrintDebug(const char* entry, ...) override;
    virtual void PrintInfo(const char* entry, ...) override;
    virtual void PrintWarning(const char* entry, ...) override;
    virtual void PrintError(const char* entry, ...) override;
    virtual void PrintFatal(const char* entry, ...) override;
private:

};
}
#endif