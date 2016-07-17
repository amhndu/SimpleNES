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

    Byte Cartridge::getNameTableMirroring()
    {
        return m_nameTableMirroring;
    }

    bool Cartridge::hasExtendedRAM()
    {
        return m_extendedRAM;
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
        std::cout << "Reading ROM from path: " << path << std::endl;

        //Header
        header.resize(0x10);
        if (!romFile.read(reinterpret_cast<char*>(&header[0]), 0x10))
        {
            std::cerr << "Reading iNES header failed." << std::endl;
            return false;
        }
        if (std::string{&header[0], &header[4]} != "NES\x1A")
        {
            std::cerr << "Not a valid iNES image. Magic number: "
                      << std::hex << header[0] << " "
                      << header[1] << " " << header[2] << " " << int(header[3]) << std::endl
                      << "Valid magic number : N E S 1a" << std::endl;
            return false;
        }

        std::cout << "Reading header, it dictates: \n";

        Byte banks = header[4];
        std::cout << "PRG-ROM Banks: " << +banks << std::endl;
        if (!banks)
        {
            std::cerr << "ROM has no PRG-ROM banks. Loading ROM failed." << std::endl;
            return false;
        }

        Byte vbanks = header[5];
        std::cout << "CHR-ROM Banks: " << +vbanks << std::endl;

        m_nameTableMirroring = header[6] & 0x9;
        std::cout << "Name Table Mirroring: " << +m_nameTableMirroring << std::endl;

        m_mapper = ((header[6] >> 4) & 0xf) | (header[7] & 0xf0);
        std::cout << "Mapper #: " << +m_mapper << std::endl;

        m_extendedRAM = header[6] & 0x2;
        std::cout << "Extended RAM: " << std::boolalpha << m_extendedRAM << std::endl;

        if (header[6] & 0x4)
        {
            std::cerr << "Trainer is not supported." << std::endl;
            return false;
        }

        if ((header[0xA] & 0x3) == 0x2 || (header[0xA] & 0x1))
        {
            std::cerr << "PAL ROM not supported." << std::endl;
            return false;
        }
        else
            std::cout << "ROM is NTSC compatible.\n";

        //PRG-ROM
        m_PRG_ROM.resize(0x4000 * banks);
        if (!romFile.read(reinterpret_cast<char*>(&m_PRG_ROM[0]), 0x4000 * banks))
        {
            std::cerr << "Reading PRG-ROM from image file failed." << std::endl;
            return false;
        }

        //CHR-ROM
        if (vbanks)
        {
            m_CHR_ROM.reserve(0x2000 * vbanks);
            if (!romFile.read(reinterpret_cast<char*>(&m_CHR_ROM[0]), 0x2000 * vbanks))
            {
                std::cerr << "Reading CHR-ROM from image file failed." << std::endl;
                return false;
            }
        }
        return true;
    }
}
