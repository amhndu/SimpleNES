#include "Mapper474.h"
#include "Log.h"

namespace sn
{
    Mapper474::Mapper474(Cartridge &cart) :
        Mapper(cart, Mapper::NROM474)
    {
        if (cart.getROM().size() == 0xC000) // 3 banks
        {
            // this is the only correct ROM size for this mapper
            m_flavor474 = true;
        }
        else
        {
            // fail
            m_flavor474 = false;
            LOG(Info) << "Incorrectly configured mapper" << std::endl;
        }

        if (cart.getVROM().size() == 0)
        {
            m_usesCharacterRAM = true;
            m_characterRAM.resize(0x2000);
            LOG(Info) << "Uses character RAM" << std::endl;
        }
        else
        {
            m_usesCharacterRAM = false;
        }
    }

    Byte MapperNROM::readPRG(Address addr)
    {
        if (m_flavor474)
        {
            return m_cartridge.getROM()[addr - 0x4000];
        }
        else
        {
            LOG(InfoVerbose) << "ROM memory read attempt while mapper is not correctly configured." << std::endl;
            return 0; // fail
        }
    }

    void MapperNROM::writePRG(Address addr, Byte value)
    {
        LOG(InfoVerbose) << "ROM memory write attempt at " << +addr << " to set " << +value << std::endl;
    }

    Byte MapperNROM::readCHR(Address addr)
    {
        if (m_usesCharacterRAM)
            return m_characterRAM[addr];
        else
            return m_cartridge.getVROM()[addr];
    }

    void MapperNROM::writeCHR(Address addr, Byte value)
    {
        if (m_usesCharacterRAM)
            m_characterRAM[addr] = value;
        else
            LOG(Info) << "Read-only CHR memory write attempt at " << std::hex << addr << std::endl;
    }
}
