#ifndef DAPMODULE_H
#define DAPMODULE_H

#pragma once

#include "Module.h"
#include "Device.h"

#include <cstdint>
#include <memory>

namespace profinet
{
    namespace dap
    {
        // Slots and subslots for DAP modules and submodules
        constexpr static uint16_t slot{                   0x00000000};
        constexpr static uint16_t subslotWholeModule{     0x00000000};
        constexpr static uint16_t subslotIdentity{        0x00000001};
        constexpr static uint16_t subslotInterface1{      0x00008000};
        constexpr static uint16_t subslotInterface1Port1{ 0x00008001};
        constexpr static uint16_t subslotInterface1Port2{ 0x00008002};
        constexpr static uint16_t subslotInterface1Port3{ 0x00008003};
        constexpr static uint16_t subslotInterface1Port4{ 0x00008004};

        // IDs for DAP modules and submodules
        constexpr static uint16_t moduleId{                   0x00000001};
        constexpr static uint16_t submoduleIdIdentity{        0x00000001};
        constexpr static uint16_t submoduleIdInterface1{      0x00008000};
        constexpr static uint16_t submoduleIdInterface1Port1{ 0x00008001};
        constexpr static uint16_t submoduleIdInterface1Port2{ 0x00008002};
        constexpr static uint16_t submoduleIdInterface1Port3{ 0x00008003};
        constexpr static uint16_t submoduleIdInterface1Port4{ 0x00008004};

        Module* CreateDapModule(Device& device);
    }
}
#endif