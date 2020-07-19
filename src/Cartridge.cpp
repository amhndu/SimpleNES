#include "Cartridge.h"
#include "Log.h"
#include <fstream>
#include <string>

namespace sn
{
    Cartridge::Cartridge() :
        m_nameTableMirroring(0),
        m_mapperNumber(0),
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
        return m_mapperNumber;
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
        std::ifstream romFile (path, std::ios_base::binary | std::ios_base::in);
        if (!romFile)
        {
            /*
             * #define LOG(level)
             */
            LOG(Error) << "Could not open ROM file from path: " << path << std::endl;
            return false;
        }

        std::vector<Byte> header;
        LOG(Info) << "Reading ROM from path: " << path << std::endl;

        //Header
        header.resize(0x10);

        if (!romFile.read(reinterpret_cast<char*>(&header[0]), 0x10))
        {
            LOG(Error) << "Reading iNES header failed." << std::endl;
            return false;
        }
        if (std::string{&header[0], &header[4]} != "NES\x1A")
        {
            LOG(Error) << "Not a valid iNES image. Magic number: "
                      << std::hex << header[0] << " "
                      << header[1] << " " << header[2] << " " << int(header[3]) << std::endl
                      << "Valid magic number : N E S 1a" << std::endl;
            return false;
        }

        LOG(Info) << "Reading header, it dictates: \n";
        /*
        https://zhuanlan.zhihu.com/p/34636695
         第 0 ~ 3 个字节指定了文件的格式，必须为：
        0 = 0x4E (N)
        1 = 0x45 (E)
        2= 0x53 (S)
        3= 0x1A (^Z)
        模拟器依靠这个确定文件的格式。

        第 4 个字节指定了 PRG（程序） ROM 块的个数，PRG ROM 块每个大小为 16KB
        第 5 个字节指定了 CHR（图块） ROM 块的个数，CHR ROM 块每个大小为 8 KB
        第 6 个字节为指定卡带属性的字节。各个比特位的含义如下：
        0 -> Mirror Type ( 1 为水平， 0 为垂直)
        1 -> 是否存在 battery-backed RAM ( 1 则为存在，映射到 $6000-$7FFF)
        2 -> 是否存在 trainer (同上，映射到 $7000-$71FF)
        3 -> 是否存在 VRAM
        4-7 -> Mapper Type 的低四位

        第 7 个字节还是指定卡带属性的字节。各个比特位的含义如下：
        *0 -> 卡带是否含有 VS-System
                        *1-3 -> 保留，但必须全为 0
        4-7 -> Mapper Type 的高四位

        第 8 个字节指定了 RAM 块的个数，每块为 8KB，如果为 0 ，则假设只有一个 RAM 块。
        *第 9 个字节指定了视频制式，如果其第 0 个比特值为 0，则为 PAL，否则为 NTSC 制式。
        第 10-15 字节为保留区域，必须为 0

        run:
        [Cartridge.cpp:50] Reading ROM from path: ../game/Super.nes
        [Cartridge.cpp:68] Reading header, it dictates:
        [Cartridge.cpp:71] 16KB PRG-ROM Banks: 2
        [Cartridge.cpp:79] 8KB CHR-ROM Banks: 1
        [Cartridge.cpp:82] Name Table Mirroring: 1
        [Cartridge.cpp:85] Mapper #: 0
        [Cartridge.cpp:88] Extended (CPU) RAM: false
        [Cartridge.cpp:102] ROM is NTSC compatible.
        */

        Byte banks = header[4];
        LOG(Info) << "16KB PRG-ROM Banks: " << +banks << std::endl;
        if (!banks)
        {
            LOG(Error) << "ROM has no PRG-ROM banks. Loading ROM failed." << std::endl;
            return false;
        }

        Byte vbanks = header[5];
        LOG(Info) << "8KB CHR-ROM Banks: " << +vbanks << std::endl;

        m_nameTableMirroring = header[6] & 0xB;
        LOG(Info) << "Name Table Mirroring: " << +m_nameTableMirroring << std::endl;

        m_mapperNumber = ((header[6] >> 4) & 0xf) | (header[7] & 0xf0);
        LOG(Info) << "Mapper #: " << +m_mapperNumber << std::endl;

        m_extendedRAM = header[6] & 0x2;
        LOG(Info) << "Extended (CPU) RAM: " << std::boolalpha << m_extendedRAM << std::endl;

        if (header[6] & 0x4)
        {
            LOG(Error) << "Trainer is not supported." << std::endl;
            return false;
        }

        if ((header[0xA] & 0x3) == 0x2 || (header[0xA] & 0x1))
        {
            LOG(Error) << "PAL ROM not supported." << std::endl;
            return false;
        }
        else
            LOG(Info) << "ROM is NTSC compatible.\n";

        //PRG-ROM 16KB banks
        m_PRG_ROM.resize(0x4000 * banks);
        if (!romFile.read(reinterpret_cast<char*>(&m_PRG_ROM[0]), 0x4000 * banks))
        {
            LOG(Error) << "Reading PRG-ROM from image file failed." << std::endl;
            return false;
        }

        //CHR-ROM 8KB banks
        if (vbanks)
        {
            m_CHR_ROM.resize(0x2000 * vbanks);
            if (!romFile.read(reinterpret_cast<char*>(&m_CHR_ROM[0]), 0x2000 * vbanks))
            {
                LOG(Error) << "Reading CHR-ROM from image file failed." << std::endl;
                return false;
            }
        }
        else
            LOG(Info) << "Cartridge with CHR-RAM." << std::endl;
        return true;
    }
}
