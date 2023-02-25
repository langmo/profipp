#include "ParameterInstance.h"
namespace profinet
{
ParameterInstance::ParameterInstance() : unknownParameter{true}, initialized{false}, lengthInBytes{0}, valueBuffer(new uint8_t[0]{})
{
}

bool ParameterInstance::Initialize(const Parameter* parameterConfiguration_)
{
    if(parameterConfiguration_)
    {
        lengthInBytes = parameterConfiguration_->GetLengthInBytes();

        const Parameter::GetCallbackType& getCallbackTmp{parameterConfiguration_->GetGetCallback()};
        const Parameter::SetCallbackType& setCallbackTmp{parameterConfiguration_->GetSetCallback()};

        if(getCallbackTmp)
        {
            getCallback = getCallbackTmp;
        }
        else
        {
            getCallback = Parameter::emptyGetCallback;
        }
        if(setCallbackTmp)
        {
            setCallback = setCallbackTmp;
        }
        else
        {
            setCallback = Parameter::emptySetCallback;
        }

        unknownParameter = false;
    }
    else
    {
        lengthInBytes = 0;

        getCallback = Parameter::emptyGetCallback;
        setCallback = Parameter::emptySetCallback;

        unknownParameter = true;
    }
    delete[] valueBuffer;
    valueBuffer = new uint8_t[lengthInBytes]{};
    initialized = true;
    return true;
}


ParameterInstance::~ParameterInstance()
{
    delete[] valueBuffer;
    valueBuffer = nullptr;
}

bool ParameterInstance::Set(const uint8_t* buffer, std::size_t numBytes)
{
    if(numBytes < lengthInBytes)
        return false;
    return setCallback(buffer, numBytes);
}
bool ParameterInstance::Get(uint8_t** buffer, std::size_t* numBytes)
{
    if(numBytes == nullptr || buffer == nullptr || *numBytes < lengthInBytes)
        return false;
    if(!getCallback(valueBuffer, lengthInBytes))
        return false;
    *buffer = valueBuffer;
    *numBytes = lengthInBytes;
    return true;
}
}