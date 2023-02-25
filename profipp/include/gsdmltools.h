#ifndef GSDMLEXPORTER_H
#define GSDMLEXPORTER_H

#pragma once

#include "Profinet.h"
#include <ostream>

namespace profinet::gsdml
{
    bool CreateGsdml(const Profinet& profinet, std::basic_ostream<char>& stream);
    bool CreateGsdml(const Profinet& profinet, std::basic_ostream<wchar_t>& stream);
    bool CreateGsdml(const Profinet& profinet, const std::string& pathToFolder);
    std::string GenerateGsdmlFileName(const Profinet& profinet);
}
#endif