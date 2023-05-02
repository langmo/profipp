#include "ZumoControl.h"
#include <iostream>
#include <string>
#include <vector>

std::string get_option(const std::vector<std::string>& args, const std::string_view& option_name, const std::string_view& default="") 
{
    for (auto it = args.begin(), end = args.end(); it != end; ++it) 
    {
        if (*it == option_name)
        {
            if (it + 1 != end)
                return *(it + 1);
        }
    }
    return default;
}
bool has_option(const std::vector<std::string>& args, const std::string_view& option_name) 
{
    for (auto it = args.begin(), end = args.end(); it != end; ++it) 
    {
        if (*it == option_name)
            return true;
    }
    
    return false;
}

int main(int argc, char *argv[])
{
    // Read in options
    const std::vector<std::string> args(argv, argv + argc);
    const bool start = has_option(args, "-s");
    const std::string filename = get_option(args, "-e");
    const std::string mainNetworkInterface = get_option(args, "-i", "wlan0");
    const std::string serial = get_option(args, "-u", "/dev/ttyAMA0")
    if (argc < 2) 
    {
        std::cout << "no options provided!\n";
        std::cout << "usage: " << argv[0] << " [-s] [-e <path>] [-i <interface>] [-u <serial>]\n";
        std::cout << "where: -s will actually start the program (otherwise only initialized)\n";
        std::cout << "       -e will export the profinet configuration as GSDML to <path>. <path> must be an existing folder.\n";
        std::cout << "       -i sets the network interface name. Defaults to wlan0.\n";
        std::cout << "       -u sets the serial/COM/uart port. Defaults to /dev/ttyAMA0.\n";
        return 0;
    }

    // Run program
    ZumoControl control{};
    bool success = control.InitializeProfinet(mainNetworkInterface);
    if(!success)
    {
        std::cerr << "Could not initialize profinet configuration.\n";
    }
    if(!start && filename.empty())
    {
        return 0;
    }
    if(!filename.empty())
    {
        control.ExportGDSML(filename.c_str());
    }
    if(start)
    {
        control.StartProfinet();
        control.RunController();
    }

}