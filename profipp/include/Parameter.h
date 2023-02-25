#ifndef PARAMETER_H
#define PARAMETER_H

#pragma once
#include "ParameterProperties.h"
#include "standardconversions.h"

#include <cstdint>
#include <functional>
#include <memory>
namespace profinet
{
class Parameter final
{
public:
    using SetCallbackType = std::function<bool(const uint8_t* buffer, std::size_t numBytes)>;
    using GetCallbackType = std::function<bool(uint8_t* buffer, std::size_t numBytes)>;
    static const SetCallbackType emptySetCallback;
    static const GetCallbackType emptyGetCallback;

    Parameter(uint16_t idx_, const SetCallbackType& setCallback_, const GetCallbackType& getCallback_, std::size_t lengthInBytes);

    uint16_t GetIdx() const;
    const SetCallbackType& GetSetCallback() const;
    const GetCallbackType& GetGetCallback() const;
    std::size_t GetLengthInBytes() const;

public:
    ParameterProperties properties;
private:

    const uint16_t idx;
    SetCallbackType setCallback;
    GetCallbackType getCallback; 
    std::size_t lengthInBytes;
};
}
#endif