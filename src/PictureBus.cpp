#include "PictureBus.h"
#include "Log.h"

namespace sn
{

    PictureBus::PictureBus() :
        m_RAM(0x800),
        m_palette(0x20),
        m_cartride(nullptr)
    {}

    Byte PictureBus::read(Address addr)
    {
        if (addr < 0x2000)
        {
            if (m_usesCharacterRAM)
                return m_characterRAM[addr];
            else
                return m_cartride->getVROM()[addr];
        }
        else if (addr < 0x3eff) //Name tables upto 0x3000, then mirrored upto 3eff
        {
            auto index = addr & 0x3ff;
            if (addr < 0x2400)      //NT0
                return m_RAM[NameTable0 + index];
            else if (addr < 0x2800) //NT1
                return m_RAM[NameTable1 + index];
            else if (addr < 0x2c00) //NT2
                return m_RAM[NameTable2 + index];
            else                    //NT3
                return m_RAM[NameTable3 + index];
        }
        else if (addr < 0x3fff)
        {
            return m_palette[addr & 0x1f];
        }
        return 0;
    }

    void PictureBus::write(Address addr, Byte value)
    {
        if (addr < 0x2000)
        {
            if (m_usesCharacterRAM)
                m_characterRAM[addr] = value;
            else
                LOG(Info) << "Read-only memory write attempt at " << std::hex << addr << std::endl;
        }
        else if (addr < 0x3eff) //Name tables upto 0x3000, then mirrored upto 3eff
        {
            auto index = addr & 0x3ff;
            if (addr < 0x2400)      //NT0
                m_RAM[NameTable0 + index] = value;
            else if (addr < 0x2800) //NT1
                m_RAM[NameTable1 + index] = value;
            else if (addr < 0x2c00) //NT2
                m_RAM[NameTable2 + index] = value;
            else                    //NT3
                m_RAM[NameTable3 + index] = value;
        }
        else if (addr < 0x3fff)
        {
            if (addr == 0x3f10) //TODO Handle palette mirroring properly (and completely)
                m_palette[0] = value;
            else
                m_palette[addr & 0x1f] = value;
       }
    }

    bool PictureBus::loadCartridge(Cartridge* cart)
    {
        if (!cart)
        {
            return false;
            LOG(Error) << "Cartride argument is nullptr" << std::endl;
        }

        m_cartride = cart;

        auto mirroring = cart->getNameTableMirroring();
//         mirroring = (mirroring & FourScreen) ? FourScreen : mirroring;
        m_mirroring = static_cast<NameTableMirroring>(mirroring);
        switch (m_mirroring)
        {
            case Horizontal:
                NameTable0 = NameTable1 = 0;
                NameTable2 = NameTable3 = 0x400;
                LOG(Info) << "Horizontal Name Table mirroring set." << std::endl;
                break;
            case Vertical:
                NameTable0 = NameTable2 = 0;
                NameTable1 = NameTable3 = 0x400;
                LOG(Info) << "Vertical Name Table mirroring set." << std::endl;
                break;
            default:
                LOG(Error) << "Unsupported Name Table mirroring." << std::endl;
                return false;
        }

        if (cart->getVROM().size() == 0)
        {
            m_usesCharacterRAM = true;
            m_characterRAM.resize(0x2000);
            LOG(Info) << "Uses character RAM" << std::endl;
        }
        else
            m_usesCharacterRAM = false;

        return true;
    }


}
