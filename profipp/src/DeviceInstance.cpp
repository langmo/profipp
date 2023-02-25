#include "DeviceInstance.h"
namespace profinet
{
DeviceInstance::DeviceInstance(): unknownDevice{true}, initialized{false}, slots{}
{
}

DeviceInstance::iterator DeviceInstance::begin()
{
    return slots.begin();
}
DeviceInstance::iterator DeviceInstance::end()
{
    return slots.end();
}

DeviceInstance::~DeviceInstance()
{

}
bool DeviceInstance::SetDefaultInputsAll()
{
    bool success{true};
    for (auto& [key, value]: slots) 
    {
        success &= value.SetDefaultInputsAll();
    }
    return success;
}
bool DeviceInstance::Initialize(const Device& deviceConfiguration_)
{
    initialized = true;
    return true;
}

ModuleInstance* DeviceInstance::CreateInSlot(uint16_t slot)
{
    auto result{slots.try_emplace(slot)};
    if(result.second)
        return &result.first->second;
    else
        return nullptr;
}

ModuleInstance* DeviceInstance::GetModule(uint16_t slot)
{
   auto it = slots.find(slot);
   if(it != slots.end())
      return &it->second;
   else
      return nullptr;
}
bool DeviceInstance::RemoveFromSlot(uint16_t slot)
{
    if(GetModule(slot)== nullptr)
        return false;
    else
    {
        slots.erase(slot);
        return true;
    }
}
}