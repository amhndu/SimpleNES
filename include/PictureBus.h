#ifndef PICTUREBUS_H
#define PICTUREBUS_H
#include <vector>
#include "Cartridge.h"

namespace sn
{
    enum NameTableMirroring
    {
        Vertical    = 0,
        Horizontal  = 1,
        FourScreen  = 1 << 3,
    };

    class PictureBus
    {
        public:
            PictureBus();
            Byte read(Address addr);
            void write(Address addr, Byte value);
            bool loadCartridge(Cartridge *cart);
        private:
            std::vector<Byte> m_RAM;
            std::size_t NameTable0, NameTable1, NameTable2, NameTable3; //indices where they start
            std::vector<Byte> m_characterRAM;
            bool m_usesCharacterRAM;
            NameTableMirroring m_mirroring;
            Cartridge* m_cartride;
    };
}
#endif // PICTUREBUS_H
