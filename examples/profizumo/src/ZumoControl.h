#ifndef ZUMOCONTROL_H
#define ZUMOCONTROL_H

#pragma once

#include "Profinet.h"
#include "zumocom.h"

class SerialConnection;

class ZumoControl final
{
public:
    ZumoControl();
    ~ZumoControl();
    
    ZumoControl (const ZumoControl&) = delete;
    ZumoControl& operator= (const ZumoControl&) = delete;

    bool InitializeProfinet();
    bool StartProfinet();
    bool ExportGDSML(const char* filename) const;
    void RunController();
private:
    profinet::LoggerType logger;
    void Log(profinet::LogLevel logLevel, const char* format, ...) noexcept;    
    bool profinetInitialized{false};
    profinet::Profinet profinet;
    
    //Outputs
    int16_t speedLeft{0};
    int16_t speedRight{0};
    
    // Inputs
    //Acceleration
    int16_t accelerationX{0};
    int16_t accelerationY{0};
    int16_t accelerationZ{0};
    // Gyro 
    int16_t gyroX{0};
    int16_t gyroY{0};
    int16_t gyroZ{0};
    // Magnetometer 
    int16_t magnetometerX{0};
    int16_t magnetometerY{0};
    int16_t magnetometerZ{0};
    // ultrasound
    int16_t distance{-1};
    // encoder
    int16_t leftEncoderCounts{0};
    int16_t rightEncoderCounts{0};
    int16_t leftEncoderCountsPerSecond{0};
    int16_t rightEncoderCountsPerSecond{0};

    // Parameters
    // encoder
    uint16_t countsPerRotation{910};
    uint16_t wheelRadius_mm{19};

    std::unique_ptr<profinet::ProfinetControl> profinetInstance;

    bool SendSerial(SerialConnection& serialConnection);
    bool ReceiveSerial(SerialConnection& serialConnection);
    bool InterpretCommand(profizumo::ZumoOutput command, int16_t value);

    static bool IsInterfaceOnline(std::string interface);
};

#endif