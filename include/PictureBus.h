#ifndef PICTUREBUS_H
#define PICTUREBUS_H
#include <vector>
#include "Cartridge.h"
#include "Mapper.h"

namespace sn
{
    class PictureBus
    {
        public:
            PictureBus();
            Byte read(Address addr);
            void write(Address addr, Byte value);

            bool setMapper(Mapper *mapper);
            Byte readPalette(Byte paletteAddr);

            void updateMirroring();
        private:
            std::vector<Byte> m_RAM;
            std::size_t NameTable0, NameTable1, NameTable2, NameTable3; //indices where they start in RAM vector

            std::vector<Byte> m_palette;

            Mapper* m_mapper;
    };
}
#endif // PICTUREBUS_H
