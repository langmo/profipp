#ifndef SUBMODUKLEINSTANCE_H
#define SUBMODUKLEINSTANCE_H

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Submodule.h"
#include "ParameterInstance.h"
#include "InputInstance.h"
#include "OutputInstance.h"
namespace profinet
{
class SubmoduleInstance final
{
public:
    SubmoduleInstance();
    ~SubmoduleInstance();
    
    SubmoduleInstance (const SubmoduleInstance&) = delete;
    SubmoduleInstance& operator= (const SubmoduleInstance&) = delete;

    bool Initialize(const Submodule& submoduleConfiguration, uint16_t subslot);
    bool InitializeUnknown(uint16_t subslot, std::size_t inputLengthInBytes_, std::size_t outputLengthInBytes_);

    ParameterInstance* GetParameter(uint16_t parameterIdx);

    std::size_t GetInputLengthInBytes();
    std::size_t GetOutputLengthInBytes();
    bool SetInput(const uint8_t* buffer, std::size_t numBytes);
    bool GetOutput(uint8_t* buffer, std::size_t* numBytes);

    bool SetDefaultInput();
private:
    bool unknownModule;
    bool initialized;
    std::map<uint16_t, ParameterInstance> parameters;
    std::vector<InputInstance> inputs;
    std::vector<OutputInstance> outputs;

    std::size_t inputLengthInBytes;
    std::size_t outputLengthInBytes;

    // initialize to PNET_IOXS_BAD=0x00 (see pnet_ioxs_values in pnet_api.h). They will be
    // (hopefully) switched to PNET_IOXS_GOOD=0x80 during the first
    // cyclic exchange.

    // The consumer status says if the controller successfully handled the last send output data.
    uint8_t lastOutputIocs{0x00};

    // The producer status says if the controller successfully sent the last input data. If not, the current input data is invalid.
    uint8_t lastInputIops{0x00};

public:
    // Status of data stream to the PLC
    uint8_t GetLastOutputIocs();
    void SetLastOutputIocs(uint8_t iocs);

    // status of data stream from the PLC
    uint8_t GetLastInputIops();
    void SetLastInputIops(uint8_t iops);
};
}
#endif