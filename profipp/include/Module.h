#ifndef PROFINETMODULE_H
#define PROFINETMODULE_H

#pragma once

#include "ModuleProperties.h"
#include "Submodule.h"
#include "helperfunctions.h"
#include <cstdint>
#include <map>
#include <memory>
namespace profinet
{
class Module final
{
public:
    using InitCallbackType = std::function<void*(uint16_t slot)>;
    /**
     * @brief Construct a new module.
     * 
     * @param id_ ID of the module.
     */
    Module(uint16_t id_);

    uint16_t GetId() const;

    ModuleProperties properties;
private:
    const uint16_t id;
    InitCallbackType initCallback;
public:
    class Submodules : public tools::MapView<uint16_t, Submodule>
    {
    public:
        Submodules() : tools::MapView<uint16_t, Submodule>{}
        {
        }
        Submodule* Create(uint16_t submoduleId);
    } submodules;

    void SetInitCallback(InitCallbackType callback_);
    const InitCallbackType& GetInitCallback() const;
};

}
#endif