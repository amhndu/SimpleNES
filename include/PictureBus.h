#ifndef PICTUREBUS_H
#define PICTUREBUS_H
#include <vector>
#include "Cartridge.h"

namespace sn
{
    class PictureBus
    {
        public:
            PictureBus();
            Byte read(Address addr);
            void write(Address addr, Byte value);
            bool loadCartridge(Cartridge *cart);
        private:
            std::vector<Byte> m_RAM;
            bool m_usesRAM;
            Cartridge* m_cartride;
    };
}
#endif // PICTUREBUS_H
