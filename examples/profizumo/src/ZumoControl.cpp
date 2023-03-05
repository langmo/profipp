#include "ZumoControl.h"
#include "SerialConnection.h"
#include "Device.h"
#include "gsdmltools.h"
#include "standardconversions.h"
#include "zumocom.h"
#include <cstdint>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>

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
    
    // Inputs
    auto leftSpeedSetCallback = [this](int16_t value) -> void
        {
            speedLeft = value;
        };
    submodule->inputs.Create<int16_t, sizeof(int16_t)>(leftSpeedSetCallback);

    auto rightSpeedSetCallback = [this](int16_t value) -> void
        {
            speedRight = value;
        };
    submodule->inputs.Create<int16_t, sizeof(int16_t)>(rightSpeedSetCallback);
    
    //Outputs
    //Acceleration
    auto accelerationXGetCallback = [this]() -> int16_t
        {
            return accelerationX;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(accelerationXGetCallback);
    auto accelerationYGetCallback = [this]() -> int16_t
        {
            return accelerationY;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(accelerationYGetCallback);
    auto accelerationZGetCallback = [this]() -> int16_t
        {
            return accelerationZ;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(accelerationZGetCallback);
    // Gyro 
    auto gyroXGetCallback = [this]() -> int16_t
        {
            return gyroX;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(gyroXGetCallback);
    auto gyroYGetCallback = [this]() -> int16_t
        {
            return gyroY;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(gyroYGetCallback);
    auto gyroZGetCallback = [this]() -> int16_t
        {
            return gyroZ;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(gyroZGetCallback);
    // Magnetometer 
    auto magnetometerXGetCallback = [this]() -> int16_t
        {
            return magnetometerX;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(magnetometerXGetCallback);
    auto magnetometerYGetCallback = [this]() -> int16_t
        {
            return magnetometerY;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(magnetometerYGetCallback);
    auto magnetometerZGetCallback = [this]() -> int16_t
        {
            return magnetometerZ;
        };
    submodule->outputs.Create<int16_t, sizeof(int16_t)>(magnetometerZGetCallback);

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

bool ZumoControl::SendSerial(SerialConnection& serialConnection)
{
    uint8_t buffer[4];
    buffer[0] = profizumo::stopByte;
    // Left motor
    buffer[1] = profizumo::FromZumoInput(profizumo::ZumoInput::leftMotorSpeed);
    profinet::toProfinet<int16_t, sizeof(int16_t)>(buffer+2, 2, speedLeft);
    if(!serialConnection.Send(buffer, 4))
    {
        return false;
    }

    // Right motor
    buffer[1] = profizumo::FromZumoInput(profizumo::ZumoInput::rightMotorSpeed);
    profinet::toProfinet<int16_t, sizeof(int16_t)>(buffer+2, 2, speedRight);
    if(!serialConnection.Send(buffer, 4))
    {
        return false;
    }
    return true;
}
bool ZumoControl::ReceiveSerial(SerialConnection& serialConnection)
{
    static uint8_t buffer[4];
    static size_t bufferPos{0};
    std::size_t numBytes{};
    while(true)
    {
        serialConnection.Read(buffer+bufferPos, 1, &numBytes);
        if(numBytes <= 0)
            break;
        else if(bufferPos == 0 && buffer[0] != profizumo::stopByte)
            continue;
        bufferPos++;
        if(bufferPos == 4)
        {
            int16_t val;
            if(profinet::fromProfinet<int16_t, sizeof(int16_t)>(buffer+2, 2, &val))
                InterpretCommand(profizumo::ToZumoOutput(buffer[1]), val);
            else
                InterpretCommand(profizumo::ZumoOutput::error, 0);
            bufferPos=0;
        }
    }
    return true;
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
        SendSerial(serialConnection);
        ReceiveSerial(serialConnection);

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
}

bool ZumoControl::InterpretCommand(profizumo::ZumoOutput command, int16_t value)
{
    switch(command)
    {
        case profizumo::ZumoOutput::accelerationX:
            accelerationX = value;
            return true;
        case profizumo::ZumoOutput::accelerationY:
            accelerationY = value;
            return true;
        case profizumo::ZumoOutput::accelerationZ:
            accelerationZ = value;
            return true;
        case profizumo::ZumoOutput::gyroX:
            gyroX = value;
            return true;
        case profizumo::ZumoOutput::gyroY:
            gyroY = value;
            return true;
        case profizumo::ZumoOutput::gyroZ:
            gyroZ = value;
            return true;
        case profizumo::ZumoOutput::magnetometerX:
            magnetometerX = value;
            return true;
        case profizumo::ZumoOutput::magnetometerY:
            magnetometerY = value;
            return true;
        case profizumo::ZumoOutput::magnetometerZ:
            magnetometerZ = value;
            return true;
        default:
            return false;
    }
}

bool ZumoControl::ExportGDSML(const char* folderName) const
{
    std::string folder{folderName};
    return profinet::gsdml::CreateGsdml(profinet, folder);
}
