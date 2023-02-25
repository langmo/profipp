#include "Parameter.h"
namespace profinet
{
const Parameter::SetCallbackType Parameter::emptySetCallback = [](const uint8_t * buffer, std::size_t numBytes) -> bool 
{
    return true;
};
const Parameter::GetCallbackType Parameter::emptyGetCallback = [](uint8_t* buffer, std::size_t numBytes) -> bool
{
    for(int i=0; i<numBytes; i++)
        buffer[i] = static_cast<uint8_t>(0);
    return false;
};

Parameter::Parameter(uint16_t idx_, const SetCallbackType& setCallback_, const GetCallbackType& getCallback_, std::size_t lengthInBytes_) : idx(idx_), setCallback(setCallback_), getCallback(getCallback_), lengthInBytes(lengthInBytes_)
{

}
uint16_t Parameter::GetIdx() const
{
    return idx;
}

const Parameter::SetCallbackType& Parameter::GetSetCallback() const
{
    return setCallback;
}
const Parameter::GetCallbackType& Parameter::GetGetCallback() const
{
    return getCallback;
}
std::size_t Parameter::GetLengthInBytes() const
{ 
    return lengthInBytes; 
}

}