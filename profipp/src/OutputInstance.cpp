#include "OutputInstance.h"
namespace profinet
{
OutputInstance::OutputInstance()  : unknownOutput{true}, lengthInBytes{0}, initialized{false}, getCallback{Output::emptyGetCallback}
{
}

bool OutputInstance::Initialize(const Output* outputConfiguration_)
{
    if(outputConfiguration_)
    {
        lengthInBytes = outputConfiguration_->GetLengthInBytes();
        const Output::GetCallbackType& getCallbackTmp{outputConfiguration_->GetGetCallback()};
        if(getCallbackTmp)
        {
            getCallback = getCallbackTmp;
        }
        else
        {
            getCallback = Output::emptyGetCallback;
        }
        unknownOutput = false;
    }
    else
    {
        lengthInBytes = 0;
        getCallback = Output::emptyGetCallback;
        unknownOutput = true;
    }
    initialized = true;
    return true;
}

OutputInstance::~OutputInstance()
{

}

bool OutputInstance::Get(uint8_t* buffer, std::size_t numBytes)
{
    if(numBytes < lengthInBytes)
        return false;
    return getCallback(buffer, numBytes);
}
std::size_t OutputInstance::GetLengthInBytes() const
{
    return lengthInBytes;
}
}