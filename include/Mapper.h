#ifndef MAPPER_H
#define MAPPER_H
#include "Cartridge.h"
#include <memory>

namespace sn
{
    enum NameTableMirroring
    {
        Horizontal  = 0,
        Vertical    = 1,
        FourScreen  = 8,
    };


    class Mapper
    {
        public:
            enum Type
            {
                NROM  = 0,
                SxROM = 1,
                UxROM = 2,
                CNROM = 3,
            };

            Mapper(Cartridge& cart, Type t) : m_cartridge(cart), m_type(t) {};
            virtual void writePRG (Address addr, Byte value) = 0;
            virtual Byte readPRG (Address addr) = 0;
            virtual const Byte* getPagePtr (Address addr) = 0; //for DMAs

            virtual Byte readCHR (Address addr) = 0;
            virtual void writeCHR (Address addr, Byte value) = 0;

            bool inline hasExtendedRAM()
            {
                return m_cartridge.hasExtendedRAM();
            }

            NameTableMirroring inline getNameTableMirroring()
            {
                return static_cast<NameTableMirroring>(m_cartridge.getNameTableMirroring());
            }

            static std::unique_ptr<Mapper> createMapper (Type mapper_t, Cartridge& cart);

        protected:
            Cartridge& m_cartridge;
            Type m_type;
    };
}

#endif //MAPPER_H
