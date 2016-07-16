#include "PictureBus.h"
#include <iostream>

namespace sn
{

    PictureBus::PictureBus() :
        m_cartride(nullptr),
        m_RAM(0x800)
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
        else if (addr < 0x3000)
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
    }

    bool PictureBus::loadCartridge(Cartridge* cart)
    {
        if (!cart)
        {
            return false;
            std::cerr << "Cartride argument is nullptr" << std::endl;
        }

        m_cartride = cart;

        auto mirroring = cart->getNameTableMirroring();
        if (mirroring & FourScreen)
            mirroring = FourScreen;
        m_mirroring = static_cast<NameTableMirroring>(mirroring);
        switch (m_mirroring)
        {
            case Vertical:
                NameTable0 = NameTable2 = 0;
                NameTable1 = NameTable3 = 0x400;
                std::cout << "Vertical Name Table mirroring set." << std::endl;
                break;
            case Horizontal:
                NameTable0 = NameTable1 = 0;
                NameTable2 = NameTable3 = 0x400;
                std::cout << "Horizontal Name Table mirroring set." << std::endl;
                break;
            default:
                std::cerr << "Unsupported Name Table mirroring." << std::endl;
                return false;
        }

        if (cart->getVROM().size() == 0)
        {
            m_characterRAM = true;
            m_characterRAM.resize(0x2000);
        }
        return true;
    }


}
