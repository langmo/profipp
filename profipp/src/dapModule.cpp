#include "dapModule.h"
namespace profinet::dap
{
Module* CreateDapModule(Device& device)
{
    auto result{device.modules.Create(moduleId, slot)};
    if(!result)
        return nullptr;
    auto& [plugInfo, module] = *result;


    // TODO: Check if these submodules don't contain any information
    module.submodules.Create(submoduleIdIdentity);
    module.submodules.Create(submoduleIdInterface1);
    module.submodules.Create(submoduleIdInterface1Port1);
    module.submodules.Create(submoduleIdInterface1Port2);
    module.submodules.Create(submoduleIdInterface1Port3);
    module.submodules.Create(submoduleIdInterface1Port4);

    return &module;    
}
}