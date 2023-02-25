#include "ConsoleLogger.h"
#include <iostream>
#include <cstdarg>
namespace profinet
{
ConsoleLogger::ConsoleLogger()
{

}

ConsoleLogger::~ConsoleLogger()
{

}

void ConsoleLogger::PrintInfo(const char* fmt, ...)
{
    va_list list;

    va_start (list, fmt);
    vprintf (fmt, list);
    va_end (list);
    fflush (stdout);
}
void ConsoleLogger::PrintWarning(const char* fmt, ...)
{
    va_list list;

    va_start (list, fmt);
    vprintf (fmt, list);
    va_end (list);
    fflush (stdout);
}
void ConsoleLogger::PrintError(const char* fmt, ...)
{
    va_list list;

    va_start (list, fmt);
    vprintf (fmt, list);
    va_end (list);
    fflush (stdout);
}

void ConsoleLogger::PrintDebug(const char* fmt, ...)
{
    va_list list;

    va_start (list, fmt);
    vprintf (fmt, list);
    va_end (list);
    fflush (stdout);
}

void ConsoleLogger::PrintFatal(const char* fmt, ...)
{
    va_list list;

    va_start (list, fmt);
    vprintf (fmt, list);
    va_end (list);
    fflush (stdout);
}
}