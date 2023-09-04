#ifndef PARAMETERROPERTIES_H
#define PARAMETERROPERTIES_H

#pragma once


#include <string>
// header provides typedefs like uint32_t
#include <cstdint>
namespace profinet
{
    struct ParameterProperties
    {
        std::string dataType{"unknown"};
        std::string name{"unknown parameter"};
        std::string description{"no description available"};
        bool visible{true};
        bool changeable{true};
        /**
         * @brief Allowed values of the parameter. If empty, all values are allowed.
         */
        std::string allowedValues{""};
        /**
         * @brief Default value for the parameter. Either, allowedValues should be empty, or defaultValue should be part of.
         */
        std::string defaultValue{"0"};
        // set to length for strings. Keep empty for non-strings.
        std::string length{""};
    };
}
#endif