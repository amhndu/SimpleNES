#include "MapperCNROM.h"
#include "Log.h"

namespace sn
{
    MapperCNROM::MapperCNROM(Cartridge &cart) :
        Mapper(cart, Mapper::CNROM),
        m_selectCHR(0)
    {
        if (cart.getROM().size() == 0x4000) //1 bank
        {
            m_oneBank = true;
        }
        else //2 banks
        {
            m_oneBank = false;
        }
    }

    Byte MapperCNROM::readPRG(Address addr)
    {
        if (!m_oneBank)
            return m_cartridge.getROM()[addr - 0x8000];
        else //mirrored
            return m_cartridge.getROM()[(addr - 0x8000) & 0x3fff];
    }

    void MapperCNROM::writePRG(Address addr, Byte value)
    {
        m_selectCHR = value & 0x3;
    }

    Byte MapperCNROM::readCHR(Address addr)
    {
        return m_cartridge.getVROM()[addr | (m_selectCHR << 13)];
    }

    void MapperCNROM::writeCHR(Address addr, Byte value)
    {
        LOG(Info) << "Read-only CHR memory write attempt at " << std::hex << addr << std::endl;
    }
}
