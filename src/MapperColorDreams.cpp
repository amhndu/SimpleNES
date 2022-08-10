#include "MapperColorDreams.h"
#include "Log.h"

namespace sn
{

    MapperColorDreams::MapperColorDreams(Cartridge &cart,std::function<void(void)> mirroring_cb) :
        Mapper(cart, Mapper::ColorDreams),
        m_mirroring(Vertical),
        m_mirroringCallback(mirroring_cb)
    {}


    Byte MapperColorDreams::readPRG(Address address)
    {
        if (address >= 0x8000)
        {
            return  m_cartridge.getROM()[(prgbank * 0x8000) + address & 0x7fff];
        }
        return 0;
    }


    void MapperColorDreams::writePRG(Address address, Byte value)
    {
        if (address >= 0x8000)
        {
            prgbank = ((value >> 0) & 0x3);
            chrbank = ((value  >> 4) & 0xF);

        }
    }


    Byte MapperColorDreams::readCHR(Address address)
    {
        if (address <= 0x1FFF)
        {
            return   m_cartridge.getVROM()[(chrbank * 0x2000) + address];
        }

        return 0;
    }


    NameTableMirroring MapperColorDreams::getNameTableMirroring()
    {
        return m_mirroring;
    }


    void MapperColorDreams::writeCHR(Address address, Byte value) {}
}
