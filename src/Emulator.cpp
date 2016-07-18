#include "Emulator.h"

namespace sn
{
    Emulator::Emulator() :
        m_cpu(m_bus),
        m_ppu(m_pictureBus, m_emulatorScreen),
        m_cycleTimer(),
        m_cpuCycleDuration(559)
    {
        m_bus.setReadCallback(PPUSTATUS, [&](void) {return m_ppu.getStatus();});
        m_bus.setReadCallback(PPUDATA, [&](void) {return m_ppu.getData();});
        m_bus.setReadCallback(OAMDATA, [&](void) {return m_ppu.getOAMData();});

        m_bus.setWriteCallback(PPUCTRL, [&](Byte b) {m_ppu.control(b);});
        m_bus.setWriteCallback(PPUMASK, [&](Byte b) {m_ppu.setMask(b);});
        m_bus.setWriteCallback(OAMADDR, [&](Byte b) {m_ppu.setOAMAddress(b);});
        m_bus.setWriteCallback(PPUADDR, [&](Byte b) {m_ppu.setDataAddress(b);});
        m_bus.setWriteCallback(PPUSCROL, [&](Byte b) {m_ppu.setScroll(b);});
        m_bus.setWriteCallback(PPUDATA, [&](Byte b) {m_ppu.setData(b);});

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

        sf::Event event;
        while (m_window.isOpen())
        {
            while (m_window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                    m_window.close();
            }

            auto elapsed_time = std::chrono::high_resolution_clock::now() - m_cycleTimer;
            m_cycleTimer = std::chrono::high_resolution_clock::now();

            while (elapsed_time > m_cpuCycleDuration)
            {
                m_ppu.step();
                m_ppu.step();
                m_ppu.step();

                m_cpu.step();
                elapsed_time -= m_cpuCycleDuration;
            }

            m_window.draw(m_emulatorScreen);
            m_window.display();
        }
    }

}