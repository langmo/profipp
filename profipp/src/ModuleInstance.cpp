#include "ModuleInstance.h"
namespace profinet
{
ModuleInstance::ModuleInstance() : initialized{false}, initArg{nullptr}, unknown{false}, subslots{}
{

}
ModuleInstance::~ModuleInstance()
{

}

ModuleInstance::iterator ModuleInstance::begin()
{
    return subslots.begin();
}
ModuleInstance::iterator ModuleInstance::end()
{
    return subslots.end();
}
bool ModuleInstance::IsUnknown() const
{
    return unknown;
}
bool ModuleInstance::Initialize(const Module& moduleConfiguration, uint16_t slot_)
{
    auto initCallback = moduleConfiguration.GetInitCallback();
    if(initCallback)
        initArg = initCallback(slot_);
    else
        initArg = nullptr;
    unknown = false;
    return true;
}
bool ModuleInstance::InitializeUnknown(uint16_t slot_)
{
    unknown = true;
    initArg = nullptr;
    initialized = true;
    return true;
}
bool ModuleInstance::SetDefaultInputsAll()
{
    bool success{true};
    for (auto& [key, value]: subslots) 
    {
        success &= value.SetDefaultInput();
    }
    return success;
}

SubmoduleInstance* ModuleInstance::CreateInSubslot(uint16_t subslot)
{
    return &(subslots.try_emplace(subslot).first->second);
}

SubmoduleInstance* ModuleInstance::GetSubmodule(uint16_t subslot)
{
   auto it = subslots.find(subslot);
   if(it != subslots.end())
      return &it->second;
   else
      return nullptr;
}

bool ModuleInstance::RemoveFromSubslot(uint16_t subslot)
{
    if(GetSubmodule(subslot)== NULL)
        return false;
    else
    {
        subslots.erase(subslot);
        return true;
    }
}
}