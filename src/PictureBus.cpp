#include "PictureBus.h"
#include <iostream>

namespace sn
{

    PictureBus::PictureBus() :
        m_cartride(nullptr)
    {
        //ctor
    }

    bool PictureBus::loadCartridge(Cartridge* cart)
    {
        if (!cart)
        {
            return false;
            std::cerr << "Cartride argument in nullptr" << std::endl;
        }

        m_cartride = cart;
        if (cart->getVROM().size() == 0)
        {
            m_usesRAM = true;
            m_RAM.resize(0x2000);
        }
        return true;
    }


}
