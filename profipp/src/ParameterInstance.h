#ifndef PARAMETERINSTANCE_H
#define PARAMETERINSTANCE_H

#pragma once
#include "Parameter.h"

namespace profinet
{
class ParameterInstance final
{
public:
    ParameterInstance();
    ~ParameterInstance();
    
    // Forbid to make copies, but move is OK.
    ParameterInstance(const ParameterInstance&) = delete;
    ParameterInstance(ParameterInstance&&) = default;
    ParameterInstance& operator= (const ParameterInstance&) = delete;
    ParameterInstance& operator= (ParameterInstance&&) = default;

    bool Set(const uint8_t* buffer, std::size_t numBytes);
    bool Get(uint8_t** buffer, std::size_t* numBytes);

    bool Initialize(const Parameter* parameterConfiguration_);

private:
    bool unknownParameter;
    bool initialized;

    Parameter::GetCallbackType getCallback;
    Parameter::SetCallbackType setCallback;

    size_t lengthInBytes;

    uint8_t* valueBuffer;
};
}

#endif