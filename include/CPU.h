#ifndef CPU_H
#define CPU_H

namespace sn
{

    using uint8_t  = Byte;
    using uint16_t = Address;

    class CPU
    {
        public:
            CPU();
            ~CPU();
            void Execute();

            //Instructions are split into five sets to make decoding easier.
            //These functions return true if they succeed
            bool ExecuteImplied(Byte opcode);
            bool ExecuteBranch(Byte opcode);
            bool ExecuteType0(Byte opcode);
            bool ExecuteType1(Byte opcode);
            bool ExecuteType2(Byte opcode);

        private:
            Byte Read(Address addr);
            void Write(Address addr, Byte value);

            //Directly access the SRAM, with no checks
            Byte ReadRAM(Address addr);
            void WriteRAM(Address addr, Byte value);

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

            Byte m_RAM[2048];
    };

};
#endif // CPU_H
