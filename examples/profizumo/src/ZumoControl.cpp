#include "ZumoControl.h"
#include "Device.h"
#include "gsdmltools.h"
#include <cstdint>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>

#include "SerialConnection.h"

ZumoControl::ZumoControl() : profinet{}, speedLeft{0}
{

}

ZumoControl::~ZumoControl()
{

}
bool ZumoControl::InitializeProfinet()
{
    profinet::Device& device = profinet.GetDevice();

    device.properties.vendorName = "FH Technikum Wien";
    // ID of the device, which is unique for the given vendor.
    device.properties.deviceID = 0x0002;
    device.properties.deviceName = "Profizumo";
    device.properties.deviceInfoText = "A small Zumo mobile robot controlled by Profinet.";
    device.properties.deviceProductFamily = "robots";
    // profinet name
    device.properties.stationName = "profizumo";        
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
    device.properties.productName = "Profizumo Robot";

    /* GSDML tag: MinDeviceInterval */
    device.properties.minDeviceInterval = 8*32; /* 8*1 ms */
    device.properties.defaultMautype = 0x10; /* Copper 100 Mbit/s Full duplex */

    auto moduleWithPlugInfo = device.modules.Create(0x00000040, 1);
    if(!moduleWithPlugInfo)
        return false;
    auto& [plugInfo, module]{*moduleWithPlugInfo};
    profinet::Submodule* submodule = module.submodules.Create(0x00000140);
    
    auto uint32SetCallback = [this](uint32_t value) -> void
        {
            speedLeft = value;
        };
    submodule->inputs.Create<uint32_t, sizeof(uint32_t)>(uint32SetCallback);
    
    profinetInitialized = true;
    return true;
}

bool ZumoControl::StartProfinet()
{
    if(!profinetInitialized)
        return false;
    profinetInstance = profinet.Initialize();
    if(!profinetInstance)
        return false;
    return profinetInstance->Start();
}

void ZumoControl::RunController()
{
    SerialConnection serialConnection{};
    if(!serialConnection.Connect())
    {
        return;
    }
    while(true)
    {
        uint8_t buffer[10];
        buffer[0] = static_cast<uint8_t>(speedLeft);
        if(!serialConnection.Send(buffer, 1))
        {
            return;
        }
        // busy waiting...
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
}

bool ZumoControl::ExportGDSML(const char* folderName) const
{
    std::string folder{folderName};
    return profinet::gsdml::CreateGsdml(profinet, folder);
}
