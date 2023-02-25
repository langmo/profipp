#ifndef MODULEROPERTIES_H
#define MODULEROPERTIES_H

#pragma once


#include <string>
// header provides typedefs like uint32_t
#include <cstdint>
namespace profinet
{
    struct ModuleProperties
    {
        std::string name{"unknown module"};
        std::string infoText{"no module information available"};
        std::string hardwareRelease{"1.0"};
        std::string softwareRelease {"1.0"};
    };
}
#endif