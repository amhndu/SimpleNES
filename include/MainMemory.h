#ifndef MEMORY_H
#define MEMORY_H
#include <vector>

namespace sn
{
    using uint8_t  = Byte;
    using uint16_t = Address;

    class MainMemory
    {
        public:
            MainMemory();
            ~MainMemory();
            Byte operator[](Address addr);
            void set(std::size_t start, std::size_t length, Byte* pointer);
        private:
            std::vector<Byte> mem;
    };
};

#endif // MEMORY_H
