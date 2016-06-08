#ifndef CPU_H
#define CPU_H
#include "MainMemory.h"

namespace sn
{

    class CPU
    {
        public:
            enum InterruptType
            {
                IRQ,
                NMI,
                BRK
            };

            CPU(MainMemory &mem);
            CPU(MainMemory &mem, Address start_addr);

            void Execute();

            //Instructions are split into five sets to make decoding easier.
            //These functions return true if they succeed
            bool ExecuteImplied(Byte opcode);
            bool ExecuteBranch(Byte opcode);
            bool ExecuteType0(Byte opcode);
            bool ExecuteType1(Byte opcode);
            bool ExecuteType2(Byte opcode);
        private:
            //Assuming sequential execution, for asynchronously calling this with Execute, further work needed
            void Reset();
            void Reset(Address start_addr);
            void Interrupt(InterruptType type);

            Byte Read(Address addr);
            Address ReadAddress(Address addr);

            void Write(Address addr, Byte value);

            void PushStack(Byte value);
            Byte PullStack();

            //If a and b are in different pages, increases the m_SkipCycles by inc
            void SetPageCrossed(Address a, Address b, int inc = 1);
            void SetZN(Byte value);
            //Returns a - b and sets flags correspondingly
            Byte SubtractAndSet(Byte a, Byte b);

            int m_SkipCycles;

            //Registers
            Address r_PC;
            Byte r_SP;
            Byte r_A;
            Byte r_X;
            Byte r_Y;

            //Status flags.
            //Is storing them in one byte better ?
            bool f_C;
            bool f_Z;
            bool f_I;
            bool f_B;
            bool f_V;
            bool f_N;

            MainMemory &m_Memory;
            //Byte m_RAM[2048];
    };

};
#endif // CPU_H
