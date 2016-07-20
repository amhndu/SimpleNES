#ifndef EMULATOR_H
#define EMULATOR_H
#include <SFML/Graphics.hpp>
#include <chrono>

#include "CPU.h"
#include "PPU.h"
#include "MainBus.h"
#include "PictureBus.h"

namespace sn
{
    using TimePoint = std::chrono::high_resolution_clock::time_point;

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

        sf::RenderWindow m_window;
        VirtualScreen m_emulatorScreen;

        TimePoint m_cycleTimer;

        std::chrono::high_resolution_clock::duration m_elapsedTime;
        std::chrono::nanoseconds m_cpuCycleDuration;
    };
}
#endif // EMULATOR_H
