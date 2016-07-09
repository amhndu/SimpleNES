#include "MainMemory.h"
#include <cstring>

namespace sn
{
    MainMemory::MainMemory() :
        mem(0x10000, 0)
    {
    }

    Byte& MainMemory::operator[](Address addr)
    {
        if (addr < 0x2000)
            return mem[addr & 0x1fff];
        return mem[addr];
    }

    void MainMemory::set(std::size_t start, std::size_t length, Byte* pointer)
    {
        std::memcpy(&mem[start], pointer, length);
    }

};
