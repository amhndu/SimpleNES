#ifndef MEMORY_H
#define MEMORY_H
#include <vector>
#include <map>
#include <functional>
#include "Cartridge.h"

enum IORegisters
{
    PPUCTRL = 0x2000,
    PPUMASK,
    PPUSTATUS,
    OAMADDR,
    OAMDATA,
    PPUSCROL,
    PPUADDR,
    PPUDATA,
    OAMDMA = 0x4014,
};

namespace sn
{

    class MainBus
    {
        public:
            MainBus();
            Byte read(Address addr);
            void write(Address addr, Byte value);
            bool loadCartridge(Cartridge *cart);
            void setWriteCallback(IORegisters reg, std::function<void(Byte)> callback);
            void setReadCallback(IORegisters reg, std::function<Byte(void)> callback);

        private:
            std::vector<Byte> m_RAM;
            std::vector<Byte> m_extRAM;
            Cartridge* m_cartride;
            Byte m_mapper;

            std::map<IORegisters, std::function<void(Byte)>> m_writeCallbacks;
            std::map<IORegisters, std::function<Byte(void)>> m_readCallbacks;
            //temporary only
            bool one_bank;
    };
};

#endif // MEMORY_H
