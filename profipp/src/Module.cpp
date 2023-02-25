#include "Module.h"
namespace profinet
{
Module::Module(uint16_t id_) : id(id_), submodules{}, properties{}
{
   
}
uint16_t Module::GetId() const
{
    return id;
}
void Module::SetInitCallback(InitCallbackType callback_)
{
   initCallback = std::move(callback_);
}
const Module::InitCallbackType& Module::GetInitCallback() const
{
   return initCallback; 
}
Submodule* Module::Submodules::Create(uint16_t submoduleId)
{
    auto result = map.try_emplace(submoduleId, submoduleId);
   if(result.second)
      return &result.first->second;
   else
      return nullptr;
}
}