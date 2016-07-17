#include <iostream>
#include "Emulator.h"
#include "Log.h"

int main()
{
    sn::Log::get().openFile("simplenes.log");
    sn::Log::get().setLevel(sn::Log::CpuTrace);
    
    sn::Emulator emulator;

    emulator.run("nestest.nes");
    return 0;
}
