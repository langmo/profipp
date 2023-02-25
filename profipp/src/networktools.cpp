#include <vector>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#else
#include <sys/socket.h> // socket(), AF_INET
#include <sys/ioctl.h> // ioctl(), SIOCGIFADDR
#include <netinet/in.h> // struct sockaddr_in
#include <net/if.h> // struct ifreq
#include <unistd.h> // close()
#include <arpa/inet.h> // inet_ntop
#endif

#include "networktools.h"
namespace profinet
{
namespace tools
{
std::map<std::string, NetworkInterface> GetNetworkInterfaces() {
  std::map<std::string, NetworkInterface> interfaces;

#ifdef _WIN32
  // TODO: Code outdated and not checked.

  // Query the interfaces on Windows
  ULONG size = 0;
  GetAdaptersAddresses(AF_INET, 0, nullptr, nullptr, &size);
  std::vector<char> buffer(size);
  auto info = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());
  if (GetAdaptersAddresses(AF_INET, 0, nullptr, info, &size) != ERROR_SUCCESS) {
    return interfaces;
  }

  // Loop through the interfaces and extract their information
  for (auto adapter = info; adapter; adapter = adapter->Next) {
    // Skip loopback and tunnel interfaces
    if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK || adapter->IfType == IF_TYPE_TUNNEL) {
      continue;
    }

    NetworkInterface interface;
    interface.name = adapter->FriendlyName;

    // Extract the IP address
    if (adapter->FirstUnicastAddress) {
      auto addr = adapter->FirstUnicastAddress->Address.lpSockaddr;
      char ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(addr)->sin_addr, ip, sizeof(ip));
      interface.ip = ip;
    }

    // Extract the subnet mask
    if (adapter->FirstPrefix) {
      auto addr = adapter->FirstPrefix->Address.lpSockaddr;
      char mask[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(addr)->sin_addr, mask, sizeof(mask));
      interface.subnet_mask = mask;
    }

    // Extract the default gateway
    for (auto gateway = adapter->FirstGatewayAddress; gateway; gateway = gateway->Next) {
      auto addr = gateway->Address.lpSockaddr;
      char gw[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(addr)->sin_addr, gw, sizeof(gw));
      interface.gateway = gw;
      break;
    }

    interfaces.push_back(interface);
  }
#else
  // Query the interfaces on POSIX systems
  // Create a socket
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    // Handle error
    return interfaces;
  }

  // Allocate memory for the interface request structure
  char buf[sizeof(struct ifreq) * 16] = { 0 };
  struct ifconf ifc;
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;

  // Get the list of interfaces
  if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
    // Handle error
    close(sock);
    return interfaces;
  }

  // Iterate through each interface and retrieve its information
  int numInterfaces = ifc.ifc_len / sizeof(struct ifreq);
  for (int i = 0; i < numInterfaces; i++) {
    struct ifreq* req = &ifc.ifc_req[i];

    // Get the IP address for the interface
    if (ioctl(sock, SIOCGIFADDR, req) < 0) {
      // Handle error
      continue;
    }
    struct sockaddr_in* addr = (struct sockaddr_in*)&req->ifr_addr;
    char ip[INET_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    uint32_t ipRaw = ntohl (addr->sin_addr.s_addr);

    // Get the subnet mask for the interface
    if (ioctl(sock, SIOCGIFNETMASK, req) < 0) {
      // Handle error
      continue;
    }
    struct sockaddr_in* subnet_mask = (struct sockaddr_in*)&req->ifr_netmask;
    char mask[INET_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET, &subnet_mask->sin_addr, mask, sizeof(mask));
    uint32_t maskRaw = ntohl (subnet_mask->sin_addr.s_addr);

    // Get the standard gateway for the interface
    if (ioctl(sock, SIOCGIFDSTADDR, req) < 0) {
      // Handle error
      continue;
    }
    struct sockaddr_in* gatewayS = (struct sockaddr_in*)&req->ifr_dstaddr;
    char gateway[INET_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET, &gatewayS->sin_addr, gateway, sizeof(gateway));
    uint32_t gatewayRaw = ntohl (gatewayS->sin_addr.s_addr);

    // Save the interface information in a struct
    NetworkInterface interface;
    interface.name = req->ifr_name;
    interface.ip = ip;
    interface.ipRaw = ipRaw;
    interface.mask = mask;
    interface.maskRaw = maskRaw;
    interface.gateway = gateway;
    interface.gatewayRaw = gatewayRaw;
    interfaces[interface.name]=interface;
  }

  // Close the socket
  close(sock);

#endif

  return interfaces;
}
}
}