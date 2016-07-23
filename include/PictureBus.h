#ifndef PICTUREBUS_H
#define PICTUREBUS_H
#include <vector>
#include "Cartridge.h"

namespace sn
{
    enum NameTableMirroring
    {
        Horizontal  = 0,
        Vertical    = 1,
        FourScreen  = 8,
    };

    class PictureBus
    {
        public:
            PictureBus();
            Byte read(Address addr);
            void write(Address addr, Byte value);

            bool loadCartridge(Cartridge *cart);
            Byte readPalette(Byte paletteAddr);
        private:
            std::vector<Byte> m_RAM;
            NameTableMirroring m_mirroring;
            std::size_t NameTable0, NameTable1, NameTable2, NameTable3; //indices where they start in RAM vector

            bool m_usesCharacterRAM;
            std::vector<Byte> m_characterRAM;
            std::vector<Byte> m_palette;

            Cartridge* m_cartride;
    };
}
#endif // PICTUREBUS_H
