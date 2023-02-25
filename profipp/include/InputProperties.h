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
        std::string description{"no description available"};
    };
}
#endif