#include "Input.h"
namespace profinet
{
const Input::SetCallbackType Input::emptySetCallback = [](const uint8_t * buffer, std::size_t numBytes) -> bool 
{
    return true;
};

Input::Input(const SetCallbackType& setCallback_, std::size_t lengthInBytes_) : setCallback(setCallback_), lengthInBytes(lengthInBytes_), properties{}
{

}

Input::~Input()
{

}

const Input::SetCallbackType& Input::GetSetCallback() const
{
    return setCallback;
}
std::size_t Input::GetLengthInBytes() const
{ 
    return lengthInBytes; 
}

}