#ifndef INPUT_H
#define INPUT_H

#pragma once
#include "InputProperties.h"
#include "standardconversions.h"

#include <cstdint>
#include <functional>
#include <memory>
namespace profinet
{
class Input final
{
public:
    using SetCallbackType = std::function<bool(const uint8_t* buffer, std::size_t numBytes)>;
    static const SetCallbackType emptySetCallback;

    Input(const SetCallbackType& setCallback_, std::size_t lengthInBytes_);
    ~Input();

    const SetCallbackType& GetSetCallback() const;
    std::size_t GetLengthInBytes() const;

public:
    InputProperties properties; 

private:
    SetCallbackType setCallback;
    std::size_t lengthInBytes;
};
}
#endif