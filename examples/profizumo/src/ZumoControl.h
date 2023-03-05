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
    bool profinetInitialized{false};
    profinet::Profinet profinet;
    int16_t speedLeft;
    int16_t speedRight;
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
    std::unique_ptr<profinet::ProfinetControl> profinetInstance;

    bool SendSerial(SerialConnection& serialConnection);
    bool ReceiveSerial(SerialConnection& serialConnection);
    bool InterpretCommand(profizumo::ZumoOutput command, int16_t value);
};

#endif