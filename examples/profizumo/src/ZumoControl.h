#ifndef ZUMOCONTROL_H
#define ZUMOCONTROL_H

#pragma once

#include "Profinet.h"

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
    uint32_t speedLeft;
    std::unique_ptr<profinet::ProfinetControl> profinetInstance;
};

#endif