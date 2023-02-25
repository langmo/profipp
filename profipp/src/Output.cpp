#include "Output.h"
namespace profinet
{
const Output::GetCallbackType Output::emptyGetCallback = [](uint8_t* buffer, std::size_t numBytes) -> bool
{
    for(int i=0; i<numBytes; i++)
        buffer[0] = static_cast<uint8_t>(0);
    return false;
};
Output::Output(GetCallbackType getCallback_, std::size_t lengthInBytes_) : getCallback(std::move(getCallback_)), lengthInBytes(lengthInBytes_), properties{}
{

}

Output::~Output()
{

}

const Output::GetCallbackType& Output::GetGetCallback() const
{
    return getCallback;
}
std::size_t Output::GetLengthInBytes() const
{ 
    return lengthInBytes; 
}
}