#ifndef FILELOGGER_H
#define FILELOGGER_H

#pragma once
#include "Logger.h"
namespace profinet
{
class FileLogger  : public Logger
{
public:
    FileLogger();
    virtual ~FileLogger();
    
    FileLogger (const FileLogger&) = delete;
    FileLogger& operator= (const FileLogger&) = delete;

    virtual void PrintDebug(const char* entry, ...) override;
    virtual void PrintInfo(const char* entry, ...) override;
    virtual void PrintWarning(const char* entry, ...) override;
    virtual void PrintError(const char* entry, ...) override;
    virtual void PrintFatal(const char* entry, ...) override;
private:
};
}

#endif