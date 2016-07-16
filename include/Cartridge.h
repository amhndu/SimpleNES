#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <vector>
#include <string>

namespace sn
{
    using Byte = uint8_t;
    using Address = uint16_t;

    class Cartridge
    {
        public:
            Cartridge();
            bool loadFromFile(std::string path);
            const std::vector<Byte>& getROM();
            const std::vector<Byte>& getVROM();
            Byte getMapper();
            Byte getNameTableMirroring();
            bool hasExtendedRAM();
        private:
            std::vector<Byte> m_PRG_ROM;
            std::vector<Byte> m_CHR_ROM;
            Byte m_nameTableMirroring;
            Byte m_mapper;
            bool m_extendedRAM;
            bool m_chrRAM;
    };

};

#endif // CARTRIDGE_H
