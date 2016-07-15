#ifndef EMULATOR_H
#define EMULATOR_H
#include "CPU.h"
#include "PPU.h"
#include "MainBus.h"
#include "PictureBus.h"

namespace sn
{
    class Emulator
    {
    public:
        Emulator();
        void run(std::string rom_path);
    private:
        MainBus m_bus;
        PictureBus m_pictureBus;
        CPU m_cpu;
        PPU m_ppu;
        Cartridge m_cartridge;

    };
}
#endif // EMULATOR_H
