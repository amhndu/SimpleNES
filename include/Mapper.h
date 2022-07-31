#ifndef MAPPER_H
#define MAPPER_H
#include "Cartridge.h"
#include <memory>
#include <functional>

namespace sn
{
    enum NameTableMirroring
    {
        Horizontal  = 0,
        Vertical    = 1,
        FourScreen  = 8,
        OneScreenLower,
        OneScreenHigher,
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
                MMC3  = 4,
                AxROM = 7,
                ColorDreams = 11,
                GxROM = 66,
            };

            Mapper(Cartridge& cart, Type t) : m_cartridge(cart), m_type(t) {};
            virtual void writePRG (Address addr, Byte value) = 0;
            virtual Byte readPRG (Address addr) = 0;

            virtual Byte readCHR (Address addr) = 0;
            virtual void writeCHR (Address addr, Byte value) = 0;

            virtual NameTableMirroring getNameTableMirroring();

            bool inline hasExtendedRAM()
            {
                return m_cartridge.hasExtendedRAM();
            }

            static std::unique_ptr<Mapper> createMapper (Type mapper_t, Cartridge& cart, std::function<void(void)> mirroring_cb);

        protected:
            Cartridge& m_cartridge;
            Type m_type;
    };
}

#endif //MAPPER_H
