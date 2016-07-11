#ifndef MEMORY_H
#define MEMORY_H
#include <vector>
#include "Cartridge.h"

namespace sn
{

    class MainMemory
    {
        public:
            MainMemory();
            Byte& operator[](Address addr);
            bool loadCartridge(Cartridge *cart);
        private:
            std::vector<Byte> m_data;
            Cartridge* m_cartride;
            Byte m_mapper;
    };
};

#endif // MEMORY_H
