#include "InputInstance.h"
namespace profinet
{

InputInstance::InputInstance() : unknownInput{true}, lengthInBytes{0}, initialized{false}, setCallback{Input::emptySetCallback}
{
    
}

bool InputInstance::Initialize(const Input* inputConfiguration_)
{
    if(inputConfiguration_)
    {
        lengthInBytes = inputConfiguration_->GetLengthInBytes();
        const Input::SetCallbackType& setCallbackTmp{inputConfiguration_->GetSetCallback()};
        if(setCallbackTmp)
        {
            setCallback = setCallbackTmp;
        }
        else
        {
            setCallback = Input::emptySetCallback;
        }
        unknownInput = false;
    }
    else
    {
        lengthInBytes = 0;
        setCallback = Input::emptySetCallback;
        unknownInput = true;
    }
    initialized = true;
    return true;
}
bool InputInstance::SetDefaultInput()
{
    // TODO: Implement
    return true;
}
InputInstance::~InputInstance()
{

}

bool InputInstance::Set(const uint8_t* buffer, std::size_t numBytes)
{
    if(numBytes < lengthInBytes)
        return false;
    return setCallback(buffer, numBytes);
}

std::size_t InputInstance::GetLengthInBytes() const
{
    return lengthInBytes;
}
}