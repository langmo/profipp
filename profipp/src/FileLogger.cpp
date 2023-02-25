#include "FileLogger.h"
#include "easylogging.h"

#include <cstdarg>

INITIALIZE_EASYLOGGINGPP
namespace profinet
{
FileLogger::FileLogger()
{

}

FileLogger::~FileLogger()
{

}

void FileLogger::PrintInfo(const char* fmt, ...)
{
    va_list list;

    char buffer[256];

    va_start (list, fmt);
    vsprintf  (buffer, fmt, list);
    va_end (list);
    fflush (stdout);

    LOG(INFO) << buffer;
}
void FileLogger::PrintWarning(const char* fmt, ...)
{
    va_list list;

    char buffer[256];

    va_start (list, fmt);
    vsprintf  (buffer, fmt, list);
    va_end (list);
    fflush (stdout);

    LOG(WARNING) << buffer;
}
void FileLogger::PrintError(const char* fmt, ...)
{
    va_list list;

    char buffer[256];

    va_start (list, fmt);
    vsprintf  (buffer, fmt, list);
    va_end (list);
    fflush (stdout);

    LOG(ERROR) << buffer;
}

void FileLogger::PrintDebug(const char* fmt, ...)
{
    va_list list;

    char buffer[256];

    va_start (list, fmt);
    vsprintf  (buffer, fmt, list);
    va_end (list);
    fflush (stdout);

    LOG(DEBUG) << buffer;
}

void FileLogger::PrintFatal(const char* fmt, ...)
{
    va_list list;

    char buffer[256];

    va_start (list, fmt);
    vsprintf  (buffer, fmt, list);
    va_end (list);
    fflush (stdout);

    LOG(FATAL) << buffer;
}
}