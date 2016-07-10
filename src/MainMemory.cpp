#include "MainMemory.h"
#include <cstring>

namespace sn
{
    MainMemory::MainMemory() :
        mem(0x10000, 0),
        m_cartride(nullptr)
    {
    }

    Byte& MainMemory::operator[](Address addr)
    {
        if (addr < 0x2000)
            return m_data[addr & 0x1fff];
        return m_data[addr];
    }

    bool MainMemory::loadCartridge(Cartridge* cart)
    {
        m_mapper = cart->getMapper();
        auto rom = cart->getROM();
        if (m_mapper != 0)
        {
            std::cerr << "Mapper not supported" << endl;
            return false;
        }

        if (rom.size() == 0x4000) //1 bank
        {
            std::memcpy(&m_data[0x8000], &rom[0], 0x4000);
            std::memcpy(&m_data[0xc000], &rom[0], 0x4000);
        }
        else //2 banks
        {
            std::memcpy(&m_data[0x8000], &rom[0], 0x4000);
            std::memcpy(&m_data[0xc000], &rom[0x4000], 0x4000);
        }
    }

};
