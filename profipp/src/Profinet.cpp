#include "Profinet.h"
#include "ProfinetInternal.h"

namespace profinet
{

Profinet::Profinet() : properties{}
{
}

Profinet::~Profinet()
{

}

std::unique_ptr<ProfinetControl> Profinet::Initialize(LoggerType logger) const
{
   auto internal{std::make_unique<ProfinetInternal>()};
   if(internal->Initialize(*this, std::move(logger)))
      return internal;
   else
      return nullptr;
}

Device& Profinet::GetDevice()
{
   return device;
}
const Device& Profinet::GetDevice() const
{
   return device;
}
ProfinetProperties& Profinet::GetProperties()
{
   return properties;
}
const ProfinetProperties& Profinet::GetProperties() const
{
   return properties;
}
template<typename T> bool CreateGSDMLInternal(const Profinet& profinet, std::basic_ostream<T>& stream)
{
   
   return true;
}
}