#include "Emulator.h"
#include "Log.h"

// #include <ctime>
// #include <sstream>
// #include <iomanip>
// #include <string>
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

    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--log-cpu") == 0)
        {
            sn::Log::get().setLevel(sn::CpuTrace);
            cpuTraceFile.open("sn.cpudump");
            sn::Log::get().setCpuTraceStream(cpuTraceFile);
            LOG(sn::Info) << "CPU logging set." << std::endl;
        }
        else
            path = argv[i];
    }

    if (path.empty())
        std::cout << "Argument required for: ROM path" << std::endl;

    sn::Emulator emulator;

    emulator.run(path);
    return 0;
}
