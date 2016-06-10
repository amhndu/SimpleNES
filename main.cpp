#include <iostream>
#include "CPU.h"
#include <fstream>

int main()
{
    sn::MainMemory memory;
    sn::CPU cpu(memory, 0xc000);
    sn::Byte rom[0x4000];

    std::ifstream testrom("nestest.nes");

    testrom.seekg(0x10);

    testrom.read(reinterpret_cast<char*>(&rom[0]), 0x4000);
    memory.set(0xc000, 0x4000, rom);

//    std::cout << "Press Enter to complete one instruction...\n";
    for (int i = 0; i < 5000; ++i)
    {
        cpu.Execute();
//        std::cout << "(0x2): " << int(memory[0x2]) << std::endl
//                  << "(0x3): " << int(memory[0x3]) << std::endl;
    }
    return 0;
}
