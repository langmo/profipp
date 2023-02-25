#ifndef DEVICEINSTANCE_H
#define DEVICEINSTANCE_H

#pragma once

#include "ModuleInstance.h"
#include "Device.h"
#include <map>

namespace profinet
{
class DeviceInstance final
{
public:
    DeviceInstance();
    ~DeviceInstance();
    
    // Forbid to make copies, but move is OK.
    DeviceInstance(const  DeviceInstance&) = delete;
    DeviceInstance( DeviceInstance&&) = default;
    DeviceInstance& operator= (const  DeviceInstance&) = delete;
    DeviceInstance& operator= ( DeviceInstance&&) = default;

    bool Initialize(const Device&  deviceConfiguration_);

    ModuleInstance* GetModule(uint16_t slot);
    ModuleInstance* CreateInSlot(uint16_t slot);
    bool RemoveFromSlot(uint16_t slot);
    bool SetDefaultInputsAll();
private:
    std::map<uint16_t,ModuleInstance> slots;
    bool unknownDevice;
    bool initialized;

public:
    using iterator = decltype(slots)::iterator;
    iterator begin();
    iterator end();
};
}
#endif