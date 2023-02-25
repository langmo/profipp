#include <iostream>
#include "EchoControl.h"

std::string get_option(const std::vector<std::string>& args, const std::string_view& option_name) 
{
    for (auto it = args.begin(), end = args.end(); it != end; ++it) 
    {
        if (*it == option_name)
        {
            if (it + 1 != end)
                return *(it + 1);
        }
    }
    return "";
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
    if (argc < 2) 
    {
        std::cout << "no options provided!\n";
        std::cout << "usage: " << argv[0] << " [-s] [-e <path>]\n";
        std::cout << "where: -s will actually start the program (otherwise only initialized)\n";
        std::cout << "       -e will export the profinet configuration as GSDML to <path>. <path> must be an existing folder.\n";
        return 0;
    }

    // Run program
    EchoControl control{};
    bool success = control.InitializeProfinet();
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