#ifndef NETWORKTOOLS_H
#define NETWORKTOOLS_H

#pragma once

#include <map>
#include <string>
namespace profinet
{
  namespace tools
  {
    struct NetworkInterface 
    {
      std::string name;
      uint32_t ipRaw;
      std::string ip;
      std::string mask;
      uint32_t maskRaw;
      std::string gateway;
      uint32_t gatewayRaw;
    };

    std::map<std::string, NetworkInterface> GetNetworkInterfaces();
  }
}
#endif // NETWORKTOOLS_H