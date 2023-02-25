#include "EchoControl.h"
#include "Device.h"
#include "gsdmltools.h"
#include <cstdint>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
EchoControl::EchoControl() : profinet(), gain(1), outputUInt32(0), outputFloat(0.0F)
{

}

EchoControl::~EchoControl()
{

}
bool EchoControl::InitializeProfinet()
{
    profinet::Device& device = profinet.GetDevice();

    device.properties.vendorName = "FH Technikum Wien";
    // ID of the device, which is unique for the given vendor.
    device.properties.deviceID = 0x0001;
    device.properties.deviceName = "Profiecho";
    device.properties.deviceInfoText = "Echos the sent in signals.";
    device.properties.deviceProductFamily = "robots";
    // profinet name
    device.properties.stationName = "profiecho";        
    device.properties.numSlots = 1;
    
    // Current software version of device.
    device.properties.swRevMajor = 0;
    device.properties.swRevMinor = 1;
    device.properties.swRevPatch = 0;
    // Current hardware version of device.
    device.properties.hwRevMajor = 1;
    device.properties.hwRevMinor = 0;

    /* Note: You need to read out the actual hardware serial number instead */
    device.properties.serialNumber = "000-000-000";

    /* GSDML tag: OrderNumber */
    device.properties.orderID = "123 444 555";

    /* GSDML tag: ModuleInfo / Name */
    device.properties.productName = "ProfiEcho";

    /* GSDML tag: MinDeviceInterval */
    device.properties.minDeviceInterval = 8*32; /* 8*1 ms */
    device.properties.defaultMautype = 0x10; /* Copper 100 Mbit/s Full duplex */

    auto moduleWithPlugInfo = device.modules.Create(0x00000040, 1);
    if(!moduleWithPlugInfo)
        return false;
    auto& [plugInfo, module]{*moduleWithPlugInfo};
    profinet::Submodule* submodule = module.submodules.Create(0x00000140);
    auto gainSetCallback = [this](const uint32_t value) -> void
        {
            gain = value;
        };
    auto gainGetCallback = [this]() -> uint32_t
        {
            return gain;
        };
    submodule->parameters.Create<uint32_t, sizeof(uint32_t)>(static_cast<uint16_t>(125), gainSetCallback, gainGetCallback, 2);
    
    
    auto floatSetCallback = [this](float value) -> void
        {
            outputFloat = value*gain;
        };
    submodule->inputs.Create<float, sizeof(float)>(floatSetCallback);

    auto floatGetCallback = [this]() -> float
        {
            return outputFloat;
        };
    submodule->outputs.Create<float, sizeof(float)>(floatGetCallback);

    auto uint32SetCallback = [this](uint32_t value) -> void
        {
            outputUInt32 = value*gain;
        };
    submodule->inputs.Create<uint32_t, sizeof(uint32_t)>(uint32SetCallback);
    auto uint32GetCallback = [this]() -> uint32_t
        {
            return outputUInt32;
        };
    submodule->outputs.Create<uint32_t, sizeof(uint32_t)>(uint32GetCallback);

    profinetInitialized = true;
    return true;
}

bool EchoControl::StartProfinet()
{
    if(!profinetInitialized)
        return false;
    profinetInstance = profinet.Initialize();
    if(!profinetInstance)
        return false;
    return profinetInstance->Start();
}

void EchoControl::RunController()
{
    while(true)
    {
        // busy waiting...
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
}

bool EchoControl::ExportGDSML(const char* folderName) const
{
    std::string folder{folderName};
    return profinet::gsdml::CreateGsdml(profinet, folder);
}
