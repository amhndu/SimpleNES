#include "Emulator.h"
#include "Log.h"

namespace sn
{
    Emulator::Emulator() :
        m_cpu(m_bus),
        m_ppu(m_pictureBus, m_emulatorScreen),
        m_cycleTimer(),
        m_cpuCycleDuration(std::chrono::nanoseconds(559))
    {
        if(!m_bus.setReadCallback(PPUSTATUS, [&](void) {return m_ppu.getStatus();}) ||
            !m_bus.setReadCallback(PPUDATA, [&](void) {return m_ppu.getData();}) ||
            !m_bus.setReadCallback(OAMDATA, [&](void) {return m_ppu.getOAMData();}))
        {
            LOG(Error) << "Critical error: Failed to set I/O callbacks" << std::endl;
        }


        if(!m_bus.setWriteCallback(PPUCTRL, [&](Byte b) {m_ppu.control(b);}) ||
            !m_bus.setWriteCallback(PPUMASK, [&](Byte b) {m_ppu.setMask(b);}) ||
            !m_bus.setWriteCallback(OAMADDR, [&](Byte b) {m_ppu.setOAMAddress(b);}) ||
            !m_bus.setWriteCallback(PPUADDR, [&](Byte b) {m_ppu.setDataAddress(b);}) ||
            !m_bus.setWriteCallback(PPUSCROL, [&](Byte b) {m_ppu.setScroll(b);}) ||
            !m_bus.setWriteCallback(PPUDATA, [&](Byte b) {m_ppu.setData(b);}))
        {
            LOG(Error) << "Critical error: Failed to set I/O callbacks" << std::endl;
        }

        m_ppu.setInterruptCallback([&](){ m_cpu.interrupt(CPU::NMI); });
    }

    void Emulator::run(std::string rom_path)
    {
        if (!m_cartridge.loadFromFile(rom_path))
            return;

        if (!m_bus.loadCartridge(&m_cartridge) ||
            !m_pictureBus.loadCartridge(&m_cartridge))
            return;

        m_cpu.reset();
        m_ppu.reset();

        m_window.create(sf::VideoMode(256 * 2, 240 * 2), "SimpleNES", sf::Style::Titlebar | sf::Style::Close);
        m_emulatorScreen.create(ScanlineVisibleDots, VisibleScanlines, 2, sf::Color::Magenta);

        m_cycleTimer = std::chrono::high_resolution_clock::now();
        m_elapsedTime = m_cycleTimer - m_cycleTimer;

        sf::Event event;
//         int step = -1;
        while (m_window.isOpen())
        {
            while (m_window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                    m_window.close();
//                 else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space && step != -1)
//                     ++step;
//                 else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::R)
//                 {
//                     //Run automatically
//                     m_cycleTimer = std::chrono::high_resolution_clock::now();
//                     step = -1;
//                 }
            }

            m_elapsedTime += std::chrono::high_resolution_clock::now() - m_cycleTimer;
            m_cycleTimer = std::chrono::high_resolution_clock::now();

            while (m_elapsedTime > m_cpuCycleDuration)
            {
                m_ppu.step();
                m_ppu.step();
                m_ppu.step();

                m_cpu.step();

//                 if (m_cpu.getPC() == 0xC3E2) //Breakpoint
//                     m_window.close();

//                 if (step != -1) --step;
                m_elapsedTime -= m_cpuCycleDuration;
            }


            m_window.draw(m_emulatorScreen);
            m_window.display();
        }
    }

}