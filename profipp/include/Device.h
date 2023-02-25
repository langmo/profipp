#ifndef DEVICE_H
#define DEVICE_H

#pragma once
#include "Module.h"
#include "DeviceProperties.h"
#include <map>
namespace profinet
{
class Device final
{
public:
    Device();
    ~Device();

    static const std::vector<uint16_t> allSlots;
public:
    struct PlugInfo final
    {
        // If empty, allowed in all slots
        const std::vector<uint16_t> allowedSlots;
        // If UINT16_MAX, not fixed
        const uint16_t fixedSlot;
        PlugInfo(const std::vector<uint16_t>& allowedSlots_=allSlots) : 
            fixedSlot{UINT16_MAX}, allowedSlots{allowedSlots_}
        {
        }
        PlugInfo(uint16_t fixedSlot_) : fixedSlot{fixedSlot_}, allowedSlots{allSlots}
        {
        }
        inline bool IsFixedSlot() const
        {
            return fixedSlot != UINT16_MAX;
        }
    };
    struct ModuleWithPlugInfo
    {
        const PlugInfo plugInfo;
        Module module;
        ModuleWithPlugInfo(uint16_t moduleID) : plugInfo{}, module{moduleID}
        {
        }
        ModuleWithPlugInfo(uint16_t moduleID, const std::vector<uint16_t>& allowedSlots) : plugInfo{allowedSlots}, module{moduleID}
        {
        }
        ModuleWithPlugInfo(uint16_t moduleID, uint16_t fixedSlot) : plugInfo{fixedSlot}, module{moduleID}
        {
        }
    };
public:
    class Modules : public tools::MapView<uint16_t, ModuleWithPlugInfo>
    {
    public:
        Modules() : tools::MapView<uint16_t, ModuleWithPlugInfo>{}
        {
        }
        ModuleWithPlugInfo* Create(uint16_t moduleId, const std::vector<uint16_t>& allowedSlots=allSlots);
        ModuleWithPlugInfo* Create(uint16_t moduleId, uint16_t fixedSlot);
    } modules; 
    
    DeviceProperties properties;   
};
}
#endif