#include "MapperMMC3.h"

namespace sn
{

    MapperMMC3::MapperMMC3(Cartridge &cart, std::function<void(void)> mirroring_cb) :
    Mapper(cart, Mapper::MMC3),
        m_targetRegister(0),
        m_prgBankMode(false),
        m_chrInversion(false),
        lastread(0),
        bIRQActive(false),
        bIRQEnable(false),
        nIRQCounter(0),
        nIRQReload(0),
        m_prgRam(32 * 1024),
        m_mirroring(Horizontal),
        m_mirroringCallback(mirroring_cb)
    {
        m_prgBank0 = &cart.getROM()[cart.getROM().size() - 0x4000];
        m_prgBank1 = &cart.getROM()[cart.getROM().size() - 0x2000];
        m_prgBank2 = &cart.getROM()[cart.getROM().size() - 0x4000];
        m_prgBank3 = &cart.getROM()[cart.getROM().size() - 0x2000];


        for (auto& bank: m_chrBanks)
        {
            bank = cart.getVROM().size() - 0x400;
        }
        m_chrBanks[0] = cart.getVROM().size() - 0x800;
        m_chrBanks[3] = cart.getVROM().size() - 0x800;
    }


    Byte MapperMMC3::readPRG(Address addr)
    {
        if (addr >= 0x6000 && addr <= 0x7FFF)
        {
            return  m_prgRam[addr & 0x1fff];
        }


        if (addr >= 0x8000 && addr <= 0x9FFF)
        {
            return *(m_prgBank0 + (addr & 0x1fff));
        }

        if (addr >= 0xA000 && addr <= 0xBFFF)
        {
            return   *(m_prgBank1 + (addr & 0x1fff));
        }

        if (addr >= 0xC000 && addr <= 0xDFFF)
        {
            return *(m_prgBank2 + (addr & 0x1fff));
        }

        if (addr >= 0xE000 && addr <= 0xFFFF)
        {
            return *(m_prgBank3  +  (addr & 0x1fff));
        }

        return 0;
    }


    Byte MapperMMC3::readCHR(Address addr)
    {
        if (addr < 0x1fff)
        {
            // select 1kb bank
            const auto bankSelect = addr >> 10;
            // get the configured base address for the bank
            const auto baseAddress = m_chrBanks[bankSelect];
            const auto offset = addr & 0x3ff;
            return m_cartridge.getVROM()[baseAddress + offset];
        }

        return 0;
    }


    void MapperMMC3::writePRG(Address addr, Byte value)
    {

        if (addr >= 0x6000 && addr <= 0x7FFF)
        {
            m_prgRam[addr & 0x1FFF] = value;
        }

        else if (addr >= 0x8000 && addr <= 0x9FFF)
        {
            // Bank Select
            if (!(addr & 0x01))
            {
                m_targetRegister = value & 0x7;
                m_prgBankMode    = value & 0x40;
                m_chrInversion   = value & 0x80;
            }
            else
            {
                m_bankRegister[m_targetRegister] = value;

                if (m_chrInversion == 0)
                {
                    // Add 0xfe mask to ignore lowest bit
                    m_chrBanks[0] = (m_bankRegister[0] & 0xFE) * 0x0400;
                    m_chrBanks[1] = (m_bankRegister[0] & 0xFE) * 0x0400 + 0x0400;
                    m_chrBanks[2] = (m_bankRegister[1] & 0xFE) * 0x0400;
                    m_chrBanks[3] = (m_bankRegister[1] & 0xFE) * 0x0400 + 0x0400;
                    m_chrBanks[4] = m_bankRegister[2] * 0x0400;
                    m_chrBanks[5] = m_bankRegister[3] * 0x0400;
                    m_chrBanks[6] = m_bankRegister[4] * 0x0400;
                    m_chrBanks[7] = m_bankRegister[5] * 0x0400;
                }
                else if (m_chrInversion == 1)
                {
                    m_chrBanks[0] = m_bankRegister[2] * 0x0400;
                    m_chrBanks[1] = m_bankRegister[3] * 0x0400;
                    m_chrBanks[2] = m_bankRegister[4] * 0x0400;
                    m_chrBanks[3] = m_bankRegister[5] * 0x0400;
                    m_chrBanks[4] = (m_bankRegister[0] & 0xFE) * 0x0400;
                    m_chrBanks[5] = (m_bankRegister[0] & 0xFE) * 0x0400 + 0x0400;
                    m_chrBanks[6] = (m_bankRegister[1] & 0xFE) * 0x0400;
                    m_chrBanks[7] = (m_bankRegister[1] & 0xFE) * 0x0400 + 0x0400;

                }

                if (m_prgBankMode == 0)
                {
                    // ignore top two bits for R6 / R7 using 0x3F
                    m_prgBank0 = &m_cartridge.getROM()[(m_bankRegister[6] & 0x3F) * 0x2000];
                    m_prgBank1 = &m_cartridge.getROM()[(m_bankRegister[7] & 0x3F) * 0x2000];
                    m_prgBank2 = &m_cartridge.getROM()[m_cartridge.getROM().size() - 0x4000];
                    m_prgBank3 = &m_cartridge.getROM()[m_cartridge.getROM().size() - 0x2000];
                }
                else if (m_prgBankMode == 1)
                {
                    m_prgBank0 = &m_cartridge.getROM()[m_cartridge.getROM().size() - 0x4000];
                    m_prgBank1 = &m_cartridge.getROM()[(m_bankRegister[7] & 0x3F) * 0x2000];
                    m_prgBank2 = &m_cartridge.getROM()[(m_bankRegister[6] & 0x3F) * 0x2000];
                    m_prgBank3 = &m_cartridge.getROM()[m_cartridge.getROM().size() - 0x2000];
                }
            }

        }
        else if (addr >= 0xA000 && addr <= 0xBFFF)
        {
            if (!(addr & 0x01))
            {
                // Mirroring
                if (value & 0x01)
                {
                    m_mirroring =  NameTableMirroring::Horizontal;
                }
                else
                {
                    m_mirroring = NameTableMirroring::Vertical;
                }
                m_mirroringCallback();
            }
            else
            {
                // PRG Ram Protect
            }
        }

        else if (addr >= 0xC000 && addr <= 0xDFFF)
        {
            if (!(addr & 0x01))
            {
                nIRQReload = value;
            }
            else
            {
                nIRQCounter = 0;
            }
        }

        else if (addr >= 0xE000 && addr <= 0xFFFF)
        {
            if (!(addr & 0x01))
            {
                bIRQEnable = false;
                bIRQActive = false;
            }
            else
            {
                bIRQEnable = true;
            }
        }
    }


    void MapperMMC3::writeCHR(Address addr, Byte value) {}


    bool MapperMMC3::irqState()
    {
        return bIRQActive;
    }

    void MapperMMC3::irqClear()
    {
        bIRQActive = false;
    }


    void MapperMMC3::scanline()
    {
        if (nIRQCounter == 0)
        {
            nIRQCounter = nIRQReload;
        }
        else
            nIRQCounter--;

        if (nIRQCounter == 0 && bIRQEnable)
        {
            bIRQActive = true;
        }
    }

    NameTableMirroring MapperMMC3::getNameTableMirroring()
    {
        return m_mirroring;
    }

} // namespace sn
