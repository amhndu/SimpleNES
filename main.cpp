#include "Emulator.h"
#include "Log.h"
#include <string>
#include <sstream>

// #include <ctime>
// #include <sstream>
// #include <iomanip>
// #include <cstring>
// #include <chrono>

/*std::string return_current_time_and_date() //courtesy of SO
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d.%X");
    return ss.str();
}*/

int main(int argc, char** argv)
{
    std::ofstream logFile ("simplenes.log"), cpuTraceFile;
    sn::TeeStream logTee (logFile, std::cout);

    if (logFile.is_open() && logFile.good())
        sn::Log::get().setLogStream(logTee);
    else
        sn::Log::get().setLogStream(std::cout);

    sn::Log::get().setLevel(sn::Info);

    std::string path;

    sn::Emulator emulator;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg (argv[i]);
        if (arg == "-h" || arg == "--help")
        {
            std::cout << "SimpleNES is a simple NES emulator.\n"
                      << "It can run off .nes images.\n\n"
                      << "Usage: SimpleNES [options] rom-path\n\n"
                      << "Options:\n"
                      << "-h, --help             Print this help text and exit\n"
                      << "-w, --width            Set the width of the emulation screen (height is\n"
                      << "                       set automatically to fit the aspect ratio)\n"
                      << "-H, --height           Set the height of the emulation screen (width is\n"
                      << "                       set automatically to fit the aspect ratio)\n"
                      << "                       This option is mutually exclusive to --width\n"
                      << std::endl;
            return 0;
        }
        else if (std::strcmp(argv[i], "--log-cpu") == 0)
        {
            sn::Log::get().setLevel(sn::CpuTrace);
            cpuTraceFile.open("sn.cpudump");
            sn::Log::get().setCpuTraceStream(cpuTraceFile);
            LOG(sn::Info) << "CPU logging set." << std::endl;
        }
        else if (std::strcmp(argv[i], "-w") == 0 || std::strcmp(argv[i], "--width") == 0)
        {
            int width;
            std::stringstream ss;
            if (i + 1 < argc && ss << argv[i + 1] && ss >> width)
                emulator.setVideoWidth(width);
            else
                LOG(sn::Error) << "Setting width from argument failed" << std::endl;
        }
        else if (std::strcmp(argv[i], "-H") == 0 || std::strcmp(argv[i], "--height") == 0)
        {
            int height;
            std::stringstream ss;
            if (i + 1 < argc && ss << argv[i + 1] && ss >> height)
                emulator.setVideoHeight(height);
            else
                LOG(sn::Error) << "Setting height from argument failed" << std::endl;
        }
        else
            path = argv[i];
    }

    if (path.empty())
    {
        std::cout << "Argument required: ROM path" << std::endl;
        return 1;
    }

    emulator.run(path);
    return 0;
}
