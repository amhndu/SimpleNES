#include <iostream>
#include "Emulator.h"

int main()
{
    sn::Emulator emulator;
    emulator.run("nestest.nes");
    return 0;
}
