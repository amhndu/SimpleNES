#ifndef MAPPERSXROM_H
#define MAPPERSXROM_H
#include "Mapper.h"

namespace sn
{
    class MapperSxROM : public Mapper
    {
        public:
            MapperSxROM(Cartridge& cart);
            void writePRG (Address addr, Byte value);
            Byte readPRG (Address addr);
            const Byte* getPagePtr(Address addr);

            Byte readCHR (Address addr);
            void writeCHR (Address addr, Byte value);
        private:
            void calculatePRGPointers();
            bool m_usesCharacterRAM;
            int m_modeCHR;
            int m_modePRG;

            Byte m_tempRegister;
            int m_writeCounter;

            Byte m_regPRG;
            Byte m_regCHR0;
            Byte m_regCHR1;

            const Byte* m_firstBankPRG;
            const Byte* m_secondBankPRG;

            const Byte* m_firstBankCHR;
            const Byte* m_secondBankCHR;

            std::vector<Byte> m_characterRAM;
    };
}
#endif // MAPPERSXROM_H
