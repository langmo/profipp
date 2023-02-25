#ifndef OUTPUT_H
#define OUTPUT_H

#pragma once
#include "standardconversions.h"
#include "OutputProperties.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace profinet
{
class Output final
{
public:
    using GetCallbackType = std::function<bool(uint8_t* buffer, std::size_t numBytes)>;
    static const GetCallbackType emptyGetCallback;
    
    Output(GetCallbackType getCallback_, std::size_t lengthInBytes_);
    ~Output();

    const GetCallbackType& GetGetCallback() const;
    std::size_t GetLengthInBytes() const;  

public:
    OutputProperties properties;

private:
    GetCallbackType getCallback;
    std::size_t lengthInBytes;
};
}
#endif