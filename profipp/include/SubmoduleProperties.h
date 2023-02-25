#ifndef SUBMODULEROPERTIES_H
#define SUBMODULEROPERTIES_H

#pragma once


#include <string>
namespace profinet
{
    struct SubmoduleProperties
    {
        std::string name{"unknown submodule"};
        std::string infoText{"no submodule information available"};
    };
}
#endif