#include "Submodule.h"
namespace profinet
{
Submodule::Submodule(uint16_t id_) : id(id_), parameters{}, inputs{}, outputs{}, properties{}
{

}
uint16_t Submodule::GetId() const
{
    return id;
}
Parameter* Submodule::Parameters::Create(uint16_t parameterIdx, const Parameter::SetCallbackType& setCallback, const Parameter::GetCallbackType& getCallback, std::size_t lengthInBytes)
{
    auto result = map.try_emplace(parameterIdx, parameterIdx, setCallback, getCallback, lengthInBytes);
    if(result.second)
       return &result.first->second;
    else
       return nullptr;
}
Input* Submodule::Inputs::Create(const Input::SetCallbackType& setCallback, std::size_t lengthInBytes)
{
    return &list.emplace_back(setCallback, lengthInBytes);
}

Output* Submodule::Outputs::Create(const Output::GetCallbackType& getCallback, std::size_t lengthInBytes)
{
    return &list.emplace_back(getCallback, lengthInBytes);
}

void Submodule::Inputs::SetAllUpdatedCallback(const Submodule::Inputs::AllUpdatedCallbackType& allUpdatedCallback_)
{
    allUpdatedCallback = allUpdatedCallback_;
}
void Submodule::Inputs::ClearAllUpdatedCallback()
{
    allUpdatedCallback = nullptr;
}
const Submodule::Inputs::AllUpdatedCallbackType& Submodule::Inputs::GetAllUpdatedCallback() const
{
    return allUpdatedCallback;
}

std::size_t Submodule::Inputs::GetLengthInBytes() const
{
    std::size_t length{0};
    for(const auto& val : list)
    {
        length += val.GetLengthInBytes();
    }
    return length;
}
std::size_t Submodule::Outputs::GetLengthInBytes() const
{
    std::size_t length{0};
    for(const auto& val : list)
    {
        length += val.GetLengthInBytes();
    }
    return length;
}
}