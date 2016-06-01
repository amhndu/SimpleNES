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
            //These patterns are observed here:
            //http://www.llx.com/~nparker/a2/opcodes.html
            //These functions return true if they succeed
            bool ExecuteImplied(Byte opcode);
            bool ExecuteBranch(Byte opcode);
            bool ExecuteType01(Byte opcode);
            bool ExecuteType10(Byte opcode);
            bool ExecuteType00(Byte opcode);

        private:
            Byte Read(Address addr);
            void Write(Address addr, Byte value);
            //Directly access the RAM, with no checks
            Byte ReadRAM(Address addr);
            void WriteRAM(Address addr, Byte value);

            void PushStack(Byte value);
            Byte PullStack();

            void SetZN(Byte value);

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

            Byte RAM[2048];
    };

};
#endif // CPU_H
