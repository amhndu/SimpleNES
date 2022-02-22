#include "MapperGxROM.h"
#include "Log.h"
namespace sn
{

    MapperGxROM::MapperGxROM(Cartridge &cart, std::function<void(void)> mirroring_cb) : 
    Mapper(cart, Mapper::GxROM),
    m_mirroringCallback(mirroring_cb),
    m_mirroring(Vertical){}

    Byte MapperGxROM::readPRG(Address address)
    {
        if (address >= 0x8000)
        {
            return m_cartridge.getROM()[(prgbank * 0x8000) + (address & 0x7fff)];
        }
    }

    void MapperGxROM::writePRG(Address address, Byte value)
    {
        int v;
        if (address >= 0x8000)
        {
            prgbank = ((value & 0x30) >> 4);
            chrbank = (value & 0x3);
            m_mirroring = Vertical;
        }
        m_mirroringCallback();
    }

    Byte MapperGxROM::readCHR(Address address)
    {
        if (address <= 0x1FFF)
        {
            return m_cartridge.getVROM()[chrbank * 0x2000 + address];
        }
    }

    NameTableMirroring MapperGxROM::getNameTableMirroring()
    {
        return m_mirroring;
    }

    void MapperGxROM::writeCHR(Address address, Byte value)
    {
        m_cartridge.getVROM()[chrbank + address, value];
    }

    const Byte *MapperGxROM::getPagePtr(Address address)
    {
    }
}
