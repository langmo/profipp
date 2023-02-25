#ifndef OUTPUTROPERTIES_H
#define OUTPUTROPERTIES_H

#pragma once


#include <string>
// header provides typedefs like uint32_t
#include <cstdint>
namespace profinet
{
    struct OutputProperties
    {
        std::string dataType{"unknown"};
        std::string description{"no description available"};
    };
}
#endif