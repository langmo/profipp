#ifndef PROFINETPROPERTIES_H
#define PROFINETPROPERTIES_H

#pragma once


#include <string>
#include <vector>
// header provides typedefs like uint32_t
#include <cstdint>
namespace profinet
{
    struct ProfinetProperties
    {
        /* Operating system specific settings */
        uint32_t  snmpThreadPriority{1};
        size_t  snmpThreadStacksize{256 * 1024}; /* bytes */
        uint32_t  ethThreadPriority{10};
        size_t  ethThreadStacksize{4096}; /* bytes */
        uint32_t  bgWorkerThreadPriority{5};
        size_t  bgWorkerThreadStacksize{4096}; /* bytes */

        // TODO: refactor
        uint32_t cycleTimerPriority{30};
        uint32_t cycleWorkerPriority{15};
        uint32_t cycleTimeUs = 1000; // 1ms

        /**
         * @brief Directory to persistantly store data. Empty string means current directory.
         * 
         */
        std::string pathStorageDirectory{""}; // current directory
        
        /* Name of the main network interface used for communication with the PLC. Typically eth0. */
        std::string mainNetworkInterface{"eth0"};
        /* Names of all network interfaces used for communication. If empty, the main network interface is 
        assumed to be the only one. If not empty, it must contain the main network interface.*/
        std::vector<std::string> networkInterfaces{};
    };
}
#endif