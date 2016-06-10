#ifndef MEMORY_H
#define MEMORY_H
#include <vector>
#include <cstdint>

namespace sn
{
    using Byte = uint8_t;
    using Address = uint16_t;

    class MainMemory
    {
        public:
            MainMemory();
            Byte& operator[](Address addr);
            void set(std::size_t start, std::size_t length, Byte* pointer);
        private:
            std::vector<Byte> mem;
    };
};

#endif // MEMORY_H
