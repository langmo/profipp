#ifndef OUTPUTINSTANCE_H
#define OUTPUTINSTANCE_H

#pragma once
#include "Output.h"
namespace profinet
{
class OutputInstance final
{
public:
    OutputInstance();
    ~OutputInstance();
    
    // Forbid to make copies, but move is OK.
    OutputInstance(const  OutputInstance&) = delete;
    OutputInstance( OutputInstance&&) = default;
    OutputInstance& operator= (const  OutputInstance&) = delete;
    OutputInstance& operator= ( OutputInstance&&) = default;

    bool Initialize(const Output* outputConfiguration_);

    bool Get(uint8_t* buffer, std::size_t numBytes);
    std::size_t GetLengthInBytes() const;
private:
    bool unknownOutput;
    bool initialized;

    Output::GetCallbackType getCallback;

    size_t lengthInBytes;

};
}
#endif