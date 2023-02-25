#ifndef ECHOCONTROL_H
#define ECHOCONTROL_H

#pragma once

#include "Profinet.h"

class EchoControl final
{
public:
    EchoControl();
    ~EchoControl();
    
    EchoControl (const EchoControl&) = delete;
    EchoControl& operator= (const EchoControl&) = delete;

    bool InitializeProfinet();
    bool StartProfinet();
    bool ExportGDSML(const char* filename) const;
    void RunController();
private:
    bool profinetInitialized{false};
    profinet::Profinet profinet;
    uint32_t gain;
    uint32_t outputUInt32;
    float outputFloat;
    std::unique_ptr<profinet::ProfinetControl> profinetInstance;
};

#endif