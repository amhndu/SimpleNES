#ifndef MAPPER474_H
#define MAPPER474_H
#include "Mapper.h"

namespace sn
{
    class Mapper474 : public Mapper
    {
        public:
            Mapper474(Cartridge& cart);
            void writePRG (Address addr, Byte value);
            Byte readPRG (Address addr);

            Byte readCHR (Address addr);
            void writeCHR (Address addr, Byte value);
        private:
            bool m_flavor474;
            bool m_usesCharacterRAM;

            std::vector<Byte> m_characterRAM;

    };
}
#endif // MAPPERNROM_H
