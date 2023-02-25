#ifndef MODULEINSTANCE_H
#define MODULEINSTANCE_H

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "SubmoduleInstance.h"
#include "Module.h"

#include <map>
namespace profinet
{
class ModuleInstance final
{
public:
   ModuleInstance();
   ~ModuleInstance();
    
   ModuleInstance (const ModuleInstance&) = delete;
   ModuleInstance& operator= (const ModuleInstance&) = delete;

   bool Initialize(const Module& moduleConfiguration_, uint16_t slot_);
   bool InitializeUnknown(uint16_t slot_);
   bool SetDefaultInputsAll();
public:
   SubmoduleInstance* CreateInSubslot(uint16_t subslot);
   SubmoduleInstance* GetSubmodule(uint16_t subslot);
   bool RemoveFromSubslot(uint16_t subslot);
   bool IsUnknown() const;

private:
   bool unknown;
   bool initialized;
   std::map<uint16_t,SubmoduleInstance> subslots;
   void* initArg;
public:
    using iterator = decltype(subslots)::iterator;
    iterator begin();
    iterator end();
};
}
#endif