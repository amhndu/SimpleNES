#include "MapperAxROM.h"
#include "Log.h"

namespace sn
{
    MapperAxROM::MapperAxROM(Cartridge &cart, std::function<void(void)> mirroring_cb) :
        Mapper(cart, Mapper::AxROM),
        m_mirroring(OneScreenLower),
        m_mirroringCallback(mirroring_cb),
        m_prgBank(0)
    {
        if (cart.getROM().size() >= 0x8000)
        {
            LOG(Info) << "Using PRG-ROM OK" << std::endl;
        }
        if (cart.getVROM().size() == 0)
        {
            m_characterRAM.resize(0x2000);
            LOG(Info) << "Uses Character RAM OK" << std::endl;
        }
    }

    Byte MapperAxROM::readPRG(Address address)
    {
        if (address >= 0x8000)
        {

            return m_cartridge.getROM()[m_prgBank * 0x8000 + (address & 0x7FFF)];
        }

        return 0;
    }

    void MapperAxROM::writePRG(Address address, Byte value)
    {
        if (address >= 0x8000)
        {
            m_prgBank = value & 0x07;
            m_mirroring = (value & 0x10) ? OneScreenHigher : OneScreenLower;
            m_mirroringCallback();
        }
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

        return 0;
    }

    void MapperAxROM::writeCHR(Address address, Byte value)
    {
        if (address < 0x2000)
        {
            m_characterRAM[address] = value;
        }
    }

}
