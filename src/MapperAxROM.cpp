#include "MapperAxROM.h"
#include "Log.h"

namespace sn
{

    MapperAxROM::MapperAxROM(Cartridge &cart, std::function<void(void)> mirroring_cb) :
    Mapper(cart, Mapper::AxROM),
    m_mirroringCallback(mirroring_cb),
    m_mirroring(OneScreenLower)   
   {
        if (cart.getROM().size() >= 0x8000)
        {
            LOG(Info) << "Using PRG-ROM OK" << std::endl;

        }

        if (cart.getVROM().size() <= 0x2000)
        {

            m_characterRAM.resize(0x2000);
            m_ppubus.m_RAM.resize(0x400);
            std::cout << "Uses Character RAM OK" << std::endl;

            for (int i = 0; i < 0x2000; i++)
            {
                m_characterRAM[i] = 0;
                //CHR RESET
            }

            for (int i = 0; i < 0x400; i++)
            {
                m_ppubus.m_RAM[i] = 0;
                //PPU RAM RESET
            }
   
        }
    }

    Byte MapperAxROM::readPRG(Address address)
    {
        if (address >= 0x8000)
        {
        
        return m_cartridge.getROM()[base + PRGBank * 0x8000 + (address & 0x7FFF)];

        }
    }

    const Byte *MapperAxROM::getPagePtr(Address address)
    {
        if (address >= 0x8000)
        {
        return &m_cartridge.getROM()[base + PRGBank * 0x8000 + (address & 0x7FFF)];
        }
    }

    void MapperAxROM::writePRG(Address address, Byte value)
    {

        if (address >= 0x8000)
        {
            PRGBank = value & 0x07;
            m_mirroring = value & 0x10 ? OneScreenHigher : OneScreenLower;
        }
         m_mirroringCallback();
    }

    NameTableMirroring MapperAxROM::getNameTableMirroring()
    {
         return m_mirroring;

    }

    Byte MapperAxROM::readCHR(Address address)
    {
        if (address < 0x2000)
        {
            return m_characterRAM[address];
        }
    }

    void MapperAxROM::writeCHR(Address address, Byte value)
    {
        if (address < 0x2000)
        {
            m_characterRAM[address] = value;
        }
    }

}
