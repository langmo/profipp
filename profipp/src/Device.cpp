#include "Device.h"
#include "dapModule.h"
namespace profinet
{
Device::Device() : properties{}
{
   dap::CreateDapModule(*this);
}

Device::~Device()
{

}
const std::vector<uint16_t> Device::allSlots{};

Device::ModuleWithPlugInfo* Device::Modules::Create(uint16_t moduleId, const std::vector<uint16_t>& allowedSlots)
{
   auto [iter, success] {map.try_emplace(moduleId, moduleId, allowedSlots)};
   if(success)
      return &iter->second;
   else
      return nullptr;
}
Device::ModuleWithPlugInfo* Device::Modules::Create(uint16_t moduleId, uint16_t fixedSlot)
{
   auto [iter, success] {map.try_emplace(moduleId, moduleId, fixedSlot)};
   if(success)
      return &iter->second;
   else
      return nullptr;
}
}