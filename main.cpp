#include "Emulator.h"
#include "Log.h"

#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>

std::string return_current_time_and_date() //courtesy of SO
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d.%X");
    return ss.str();
}

int main()
{
    sn::Log::get().setLogFile("simplenes.log"/* + return_current_time_and_date()*/);
    sn::Log::get().setCpuTraceFile("sn.cpudump"/* + return_current_time_and_date()*/);
    sn::Log::get().setLevel(sn::CpuTrace);

    sn::Emulator emulator;

    emulator.run("nestest.nes");
    return 0;
}
