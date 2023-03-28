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
#include <cstdarg>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>

ZumoControl::ZumoControl() : profinet{}, speedLeft{0}, speedRight{0}, logger{profinet::logging::CreateConsoleLogger()}
{

}

ZumoControl::~ZumoControl()
{

}

void ZumoControl::Log(profinet::LogLevel logLevel, const char* format, ...) noexcept
{
   if(!logger)
      return;
   va_list args;
   std::string message;

   va_start (args, format);
   message.resize (vsnprintf (0, 0, format, args));
   vsnprintf (&message[0], message.size () + 1, format, args);
   va_end (args);
   logger(logLevel, std::move(message));
}

bool ZumoControl::InitializeProfinet()
{
    profinet.GetProperties().mainNetworkInterface = "wlan0";
    profinet::Device& device = profinet.GetDevice();

    device.properties.vendorName = "FH Technikum Wien";
    // ID of the device, which is unique for the given vendor.
    device.properties.deviceID = 0x0002;
    device.properties.deviceName = "Profizumo";
    device.properties.deviceInfoText = "A small Zumo mobile robot controlled by Profinet.";
    device.properties.deviceProductFamily = "robots";
    // profinet name
    device.properties.stationName = "profizumo";        
    device.properties.numSlots = 3;
    
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

    // Motor module
    auto motorModuleWithPlugInfo = device.modules.Create(0x00000040, 1);
    if(!motorModuleWithPlugInfo)
        return false;
    auto& [motorPlugInfo, motorModule]{*motorModuleWithPlugInfo};
    motorModule.properties.name = "Motors";
    motorModule.properties.infoText = "Moule allows control of the two motors of the Zumu bot.";
    profinet::Submodule* motorSubmodule = motorModule.submodules.Create(0x00000140);
    motorSubmodule->properties.name = "Motors submodule";
    motorModule.properties.infoText = "Submoule allows control of the two motors of the Zumu bot.";
    // Inputs
    auto leftSpeedSetCallback = [this](int16_t value) -> void
        {
            speedLeft = value;
        };
    profinet::Input* left = motorSubmodule->inputs.Create<int16_t, sizeof(int16_t)>(leftSpeedSetCallback);
    left->properties.description = "Speed of left motor.";

    auto rightSpeedSetCallback = [this](int16_t value) -> void
        {
            speedRight = value;
        };
    profinet::Input* right = motorSubmodule->inputs.Create<int16_t, sizeof(int16_t)>(rightSpeedSetCallback);
    right->properties.description = "Speed of right motor.";
    
    // IMU module
    auto imuModuleWithPlugInfo = device.modules.Create(0x00000041, std::vector<uint16_t>{2});
    if(!imuModuleWithPlugInfo)
        return false;
    auto& [imuPlugInfo, imuModule]{*imuModuleWithPlugInfo};
    imuModule.properties.name = "IMU sensors";
    imuModule.properties.infoText = "Moule allows reading of the different IMU sensors.";
    //Acceleration
    profinet::Submodule* accelerationSubmodule = imuModule.submodules.Create(0x00000151);
    accelerationSubmodule->properties.name = "Acceleration";
    accelerationSubmodule->properties.infoText = "Acceleration sensor";
    auto accelerationXGetCallback = [this]() -> int16_t
        {
            return accelerationX;
        };
    profinet::Output* output = accelerationSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(accelerationXGetCallback);
    output->properties.description = "Acceleration in X direction";
    auto accelerationYGetCallback = [this]() -> int16_t
        {
            return accelerationY;
        };
    output = accelerationSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(accelerationYGetCallback);
    output->properties.description = "Acceleration in Y direction";
    auto accelerationZGetCallback = [this]() -> int16_t
        {
            return accelerationZ;
        };
    output = accelerationSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(accelerationZGetCallback);
    output->properties.description = "Acceleration in Z direction";
    // Gyro 
    profinet::Submodule* gyroSubmodule = imuModule.submodules.Create(0x00000152);
    gyroSubmodule->properties.name = "Gyro";
    gyroSubmodule->properties.infoText = "Gyro sensor";
    auto gyroXGetCallback = [this]() -> int16_t
        {
            return gyroX;
        };
    output = gyroSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(gyroXGetCallback);
    output->properties.description = "Gyro in X direction";
    auto gyroYGetCallback = [this]() -> int16_t
        {
            return gyroY;
        };
    output = gyroSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(gyroYGetCallback);
    output->properties.description = "Gyro in Y direction";
    auto gyroZGetCallback = [this]() -> int16_t
        {
            return gyroZ;
        };
    output = gyroSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(gyroZGetCallback);
    output->properties.description = "Gyro in Z direction";
    // Magnetometer 
    profinet::Submodule* magnetometerSubmodule = imuModule.submodules.Create(0x00000153);
    magnetometerSubmodule->properties.name = "Magnetometer";
    magnetometerSubmodule->properties.infoText = "Magnetormeter sensor";
    auto magnetometerXGetCallback = [this]() -> int16_t
        {
            return magnetometerX;
        };
    output = magnetometerSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(magnetometerXGetCallback);
    output->properties.description = "Magnetometer in X direction";
    auto magnetometerYGetCallback = [this]() -> int16_t
        {
            return magnetometerY;
        };
    output = magnetometerSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(magnetometerYGetCallback);
    output->properties.description = "Magnetometer in Y direction";
    auto magnetometerZGetCallback = [this]() -> int16_t
        {
            return magnetometerZ;
        };
    output = magnetometerSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(magnetometerZGetCallback);
    output->properties.description = "Magnetometer in Z direction";

    // Ultrasound module
    auto ultrasoundModuleWithPlugInfo = device.modules.Create(0x00000042, std::vector<uint16_t>{3});
    if(!ultrasoundModuleWithPlugInfo)
        return false;
    auto& [ultrasoundPlugInfo, ultrasoundModule]{*ultrasoundModuleWithPlugInfo};
    ultrasoundModule.properties.name = "Ultrasound distance sensor";
    ultrasoundModule.properties.infoText = "Distances are measured in mm.";
    profinet::Submodule* ultrasoundSubmodule = ultrasoundModule.submodules.Create(0x00000142);
    ultrasoundSubmodule->properties.name = "Ultrasound distance sensor";
    ultrasoundSubmodule->properties.infoText = "Distances are measured in mm.";
    //Outputs
    //distance in mm
    auto distanceGetCallback = [this]() -> int16_t
        {
            return distance;
        };
    output = ultrasoundSubmodule->outputs.Create<int16_t, sizeof(int16_t)>(distanceGetCallback);
    output->properties.description = "Distance measured by ultrasound sensor in mm.";

    profinetInitialized = true;
    return true;
}

bool ZumoControl::IsInterfaceOnline(std::string interface)
{
    struct ifreq ifr;
    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
   {
      return false;
   }
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface.c_str());
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) 
    {
            return false;
    }
    close(sock);
    return (ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING);
}

bool ZumoControl::StartProfinet()
{
    if(!profinetInitialized)
    {
        Log(profinet::logError, "Profinet not yet initialized.");
        return false;
    }
    while(!IsInterfaceOnline(profinet.GetProperties().mainNetworkInterface))
    {
        Log(profinet::logWarning, "Network interface %s not yet running. Retrying in 5s...", profinet.GetProperties().mainNetworkInterface.c_str());
        std::this_thread::sleep_for (std::chrono::seconds(5));
    }
    profinetInstance = profinet.Initialize(logger);
    if(!profinetInstance)
    {
        Log(profinet::logError, "Could not initialize Profinet.");
        return false;
    }
    return profinetInstance->Start();
}

bool ZumoControl::SendSerial(SerialConnection& serialConnection)
{
    uint8_t buffer[4];
    // Left motor
    buffer[0] = profizumo::FromZumoInput(profizumo::ZumoInput::leftMotorSpeed);
    profinet::toProfinet<int16_t, sizeof(int16_t)>(buffer+1, 2, speedLeft);
    buffer[3] = profizumo::stopByte;
    if(!serialConnection.Send(buffer, 4))
    {
        return false;
    }

    // Right motor
    buffer[0] = profizumo::FromZumoInput(profizumo::ZumoInput::rightMotorSpeed);
    profinet::toProfinet<int16_t, sizeof(int16_t)>(buffer+1, 2, speedRight);
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
    while(!serialConnection.Connect())
    {
        Log(profinet::logError, "Could not establish serial connection. Retrying in 5s...");
        std::this_thread::sleep_for (std::chrono::seconds(5));
    }
    while(true)
    {
        SendSerial(serialConnection);
        ReceiveSerial(serialConnection);

        using namespace std::chrono_literals;
        // TODO: Check if we need to sleep at all.
        std::this_thread::sleep_for(50ms);
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
        case profizumo::ZumoOutput::ultrasoundDistance:
            distance = value;
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
