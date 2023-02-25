#ifndef INPUTINSTANCE_H
#define INPUTINSTANCE_H

#pragma once
#include "Input.h"
namespace profinet
{
class InputInstance final
{
public:
    InputInstance();
    ~InputInstance();
    
    // Forbid to make copies, but move is OK.
    InputInstance (const InputInstance&) = delete;
    InputInstance (InputInstance&&) = default;
    InputInstance& operator= (const InputInstance&) = delete;
    InputInstance& operator= (InputInstance&&) = default;

    bool Initialize(const Input* inputConfiguration_);

    bool Set(const uint8_t* buffer, std::size_t numBytes);
    std::size_t GetLengthInBytes() const;

    bool SetDefaultInput();

private:
    bool unknownInput;
    bool initialized;

    Input::SetCallbackType setCallback;

    size_t lengthInBytes;
};
}
#endif