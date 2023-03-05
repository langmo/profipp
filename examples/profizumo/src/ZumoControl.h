#ifndef ZUMOCONTROL_H
#define ZUMOCONTROL_H

#pragma once

#include "Profinet.h"

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
    std::unique_ptr<profinet::ProfinetControl> profinetInstance;

    bool SendSerial(SerialConnection& serialConnection);
    bool ReceiveSerial(SerialConnection& serialConnection);
    bool InterpretCommand(profizumo::ZumoOutput command, int16_t value)
};

#endif