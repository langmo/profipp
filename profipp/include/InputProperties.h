#ifndef INPUTROPERTIES_H
#define INPUTROPERTIES_H

#pragma once


#include <string>
// header provides typedefs like uint32_t
#include <cstdint>
namespace profinet
{
    struct InputProperties
    {
        std::string dataType{"unknown"};
        // set to length for strings. Keep empty for non-strings.
        std::string length{""};
        std::string description{"no description available"};
    };
}
#endif