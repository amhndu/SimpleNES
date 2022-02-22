#ifndef MAPPERCNROM_H
#define MAPPERCNROM_H
#include "Mapper.h"

namespace sn
{
    class MapperCNROM : public Mapper
    {
        public:
            MapperCNROM(Cartridge& cart);
            void writePRG (Address addr, Byte value);
            Byte readPRG (Address addr);
            const Byte* getPagePtr(Address addr);

            Byte readCHR (Address addr);
            void writeCHR (Address addr, Byte value);
        private:
            bool m_oneBank;

            Address m_selectCHR;
    };
}
#endif // MAPPERCNROM_H
