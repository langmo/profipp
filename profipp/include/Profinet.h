#ifndef PROFINET_H
#define PROFINET_H

#pragma once

#include "ProfinetProperties.h"
#include "Device.h"
#include "logging.h"
#include <map>

namespace profinet
{
class ProfinetControl
{
public:
    virtual bool Start() = 0;
};

class Profinet final
{
public:
    Profinet();
    ~Profinet();

    std::unique_ptr<ProfinetControl> Initialize(LoggerType logger = logging::CreateConsoleLogger()) const;
public:
    Device& GetDevice();
    const Device& GetDevice() const;
    ProfinetProperties& GetProperties();    
    const ProfinetProperties& GetProperties() const;
private:
    Device device;
    ProfinetProperties properties;    
};
}
#endif