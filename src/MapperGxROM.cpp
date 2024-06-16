#include "MapperGxROM.h"
#include "Log.h"
namespace sn
{

    MapperGxROM::MapperGxROM(Cartridge &cart, std::function<void(void)> mirroring_cb) :
        Mapper(cart, Mapper::GxROM),
        m_mirroring(Vertical),
        m_mirroringCallback(mirroring_cb)
    {}

    Byte MapperGxROM::readPRG(Address address)
    {
        if (address >= 0x8000)
        {
            return m_cartridge.getROM()[(prgbank * 0x8000) + (address & 0x7fff)];
        }

        return 0;
    }

    void MapperGxROM::writePRG(Address address, Byte value)
    {
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
        return 0;
    }

    NameTableMirroring MapperGxROM::getNameTableMirroring()
    {
        return m_mirroring;
    }

    void MapperGxROM::writeCHR(Address, Byte)
    {
        LOG(Info) << "not expecting writes here";
    }
}
