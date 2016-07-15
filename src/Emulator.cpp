#include "Emulator.h"

namespace sn
{
    Emulator::Emulator() :
        m_cpu(m_bus),
        m_ppu(m_pictureBus)
    {
        m_bus.setReadCallback(PPUSTATUS, [&m_ppu](void) {return m_ppu.getStatus();});
        m_bus.setReadCallback(PPUDATA, [&m_ppu](void) {return m_ppu.getData();});
        m_bus.setReadCallback(OAMDATA, [&m_ppu](void) {return m_ppu.getOAMData();});

        m_bus.setWriteCallback(PPUCTRL, [&m_ppu](Byte b) {m_ppu.control(b);});
        m_bus.setWriteCallback(PPUMASK, [&m_ppu](Byte b) {m_ppu.setMask(b);});
        m_bus.setWriteCallback(OAMADDR, [&m_ppu](Byte b) {m_ppu.setOAMAddress(b);});
        m_bus.setWriteCallback(PPUSCROL, [&m_ppu](Byte b) {m_ppu.setScroll(b);});
        m_bus.setWriteCallback(PPUDATA, [&m_ppu](Byte b) {m_ppu.setData(b);});

    }

    void Emulator::run(std::string rom_path)
    {
        if (!m_cartridge.loadFromFile(rom_path))
            return;

        if (m_bus.loadCartridge(&m_cartridge) &&
            m_pictureBus.loadCartridge(&m_cartridge))
            return;
    }

}