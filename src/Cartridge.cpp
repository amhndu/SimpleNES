#include "Cartridge.h"
#include <fstream>
#include <iostream>
#include <string>

namespace sn
{
    Cartridge::Cartridge() :
        m_nameTableMirroring(0),
        m_mapper(0),
        m_extendedRAM(false)
    {

    }
    const std::vector<Byte>& Cartridge::getROM()
    {
        return m_PRG_ROM;
    }

    const std::vector<Byte>& Cartridge::getVROM()
    {
        return m_CHR_ROM;
    }

    Byte Cartridge::getMapper()
    {
        return m_mapper;
    }

    bool Cartridge::loadFromFile(std::string path)
    {
        std::ifstream romFile (path);
        if (!romFile)
        {
            std::cerr << "Could not open ROM file" << std::endl;
            return false;
        }

        std::vector<Byte> header;

        //Header
        header.resize(0x10);
        if (!romFile.read(&header[0], 0x10))
        {
            std::cerr << "Reading iNES header failed." << std::endl;
            return false;
        }
        if (std::string{&header[0], &header[3]} != "NES\x1A")
        {
            std::cerr << "Not a valid iNES image.\n" << std::endl;
            return false;
        }
        Byte banks = header[4];
        if (!banks)
        {
            std::cerr << "ROM has no PRG-ROM banks. Loading ROM failed." << std::endl;
            return false;
        }
        Byte vbanks = header[5];
        m_nameTableMirroring = header[6] & 0x9;
        m_mapper = ((header[6] >> 4) & 0xf) | (header[7] & 0xf0);
        m_extendedRAM = header[6] & 0x2; //we don't care
        if (header[6] & 0x4)
        {
            std::cerr << "Trainer is not supported." << std::endl;
            return false;
        }

        //PRG-ROM
        m_PRG_ROM.resize(0x4000 * banks);
        if (!romFile.read(&m_PRG_ROM[0], 0x4000 * banks))
        {
            std::cerr << "Reading PRG-ROM from image file failed." << std::endl;
            return false;
        }

        //CHR-ROM
        if (vbanks)
        {
            m_CHR_ROM.reserve(0x2000 * vbanks);
            if (!romFile.read(&m_CHR_ROM[0], 0x2000 * vbanks))
            {
                std::cerr << "Reading CHR-ROM from image file failed." << std::endl;
                return false;
            }
        }
    }

}
