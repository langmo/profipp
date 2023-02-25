#include "SubmoduleInstance.h"
namespace profinet
{
SubmoduleInstance::SubmoduleInstance() : unknownModule{true}, inputLengthInBytes(0), outputLengthInBytes(0)
{
    
}

bool SubmoduleInstance::Initialize(const Submodule& submoduleConfiguration, uint16_t subslot)
{
    for(const auto& elem : submoduleConfiguration.parameters)
    {
        auto insert = parameters.try_emplace(elem.GetIdx());
        if(!insert.second)
        {
            return false;
        }
        if(!insert.first->second.Initialize(&elem))
        {
            return false;
        }
    }
    inputLengthInBytes = 0;
    for(const auto& elem : submoduleConfiguration.inputs)
    {
        auto& input = inputs.emplace_back();
        if(!input.Initialize(&elem))
        {
            return false;
        }
        inputLengthInBytes += elem.GetLengthInBytes();
    }
    outputLengthInBytes = 0;
    for(const auto& elem : submoduleConfiguration.outputs)
    {
        auto& output = outputs.emplace_back();
        if(!output.Initialize(&elem))
        {
            return false;
        }
        outputLengthInBytes += elem.GetLengthInBytes();
    }

    unknownModule = false;
    
    initialized = true;
    return true;
}

bool SubmoduleInstance::InitializeUnknown(uint16_t subslot, std::size_t inputLengthInBytes_, std::size_t outputLengthInBytes_)
{
    inputLengthInBytes = inputLengthInBytes_;
    outputLengthInBytes = outputLengthInBytes_;
    unknownModule = true;

    initialized = true;
    return true;
}

uint8_t SubmoduleInstance::GetLastOutputIocs()
{
    return lastOutputIocs;
}
void SubmoduleInstance::SetLastOutputIocs(uint8_t iocs)
{
    lastOutputIocs = iocs;
}
uint8_t SubmoduleInstance::GetLastInputIops()
{
    return lastInputIops;
}
void SubmoduleInstance::SetLastInputIops(uint8_t iops)
{
    lastInputIops = iops;
}
bool SubmoduleInstance::SetDefaultInput()
{
    bool success{true};
    for(auto& input : inputs)
    {
        success &= input.SetDefaultInput();
    }
    return success;
}
std::size_t SubmoduleInstance::GetInputLengthInBytes()
{
    return inputLengthInBytes;
}
std::size_t SubmoduleInstance::GetOutputLengthInBytes()
{
    return outputLengthInBytes;
}

SubmoduleInstance::~SubmoduleInstance()
{
}
ParameterInstance* SubmoduleInstance::GetParameter(uint16_t parameterIdx)
{
   auto it = parameters.find(parameterIdx);
   if(it != parameters.end())
      return &it->second;
   else
      return nullptr;
}

bool SubmoduleInstance::SetInput(const uint8_t* buffer, std::size_t numBytes)
{
    if(buffer == nullptr || numBytes < inputLengthInBytes)
    {
        return false;
    }
    for(auto& input : inputs)
    {
        std::size_t length = input.GetLengthInBytes();
        bool result = input.Set(buffer, numBytes);
        if(!result)
            return false;
        buffer+=length;
        numBytes -= length;
    }
    return true;
}
bool SubmoduleInstance::GetOutput(uint8_t* buffer, std::size_t* numBytes)
{
    if(numBytes == nullptr || buffer == nullptr || *numBytes < outputLengthInBytes)
    {
        return false;
    }
    auto tempBuffer = buffer;
    size_t tempLength = outputLengthInBytes;
    for(auto& output : outputs)
    {
        std::size_t length = output.GetLengthInBytes();
        if(tempLength < length)
        {
            return false;
        }
        bool result = output.Get(tempBuffer, length);
        if(!result)
        {
            return false;
        }
        tempBuffer+=length;
        tempLength-= length;
    }
    *numBytes = outputLengthInBytes;
    return true;
}
}