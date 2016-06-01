#ifndef CPUOPCODES_H_INCLUDED
#define CPUOPCODES_H_INCLUDED

namespace sn
{
    enum Opcode01
    {
        ORA = 0x00;
        AND = 0x20;
        EOR = 0x40;
        ADC = 0x60;
        STA = 0x80;
        LDA = 0xa0;
        CMP = 0xc0;
        SBC = 0xe0;
    };
    enum Mode01
    {
        IndexedIndirectX = 0x00;
        ZeroPage         = 0x04;
        Immediate        = 0x08;
        Absolute         = 0x0c;
        IndirectY        = 0x10;
        IndexedX         = 0x14;
        AbsoluteY        = 0x18;
        AbsoluteX        = 0x1c;
    };

    enum Opcode00
    {
        ASL = 0x00;
        ROL = 0x20;
        LSR = 0x40;
        ROR = 0x60;
        STX = 0x80;
        LDX = 0xa0;
        DEC = 0xc0;
        INC = 0xe0;
    };
    enum Mode10
    {
        Immediate       = 0x00;
        ZeroPage        = 0x04;
        Accumulator     = 0x08;
        Absolute        = 0x0c;
        Indexed         = 0x14;
        AbsoluteIndexed = 0x1c;
    };

    enum Opcode10
    {
        BIT  = 0x20;
        JMP  = 0x40;
        JMPA = 0x60;
        STY  = 0x80;
        LDY  = 0xa0;
        CPY  = 0xc0;
        CPX  = 0xe0;
    };
    enum Mode10
    {
        Immediate       = 0x00;
        ZeroPage        = 0x04;
        Absolute        = 0x0c;
        Indexed         = 0x14;
        AbsoluteIndexed = 0x1c;
    };

    enum OpcodeImplied
    {
        NOP = 0xea;
        BRK = 0x00;
        JSR = 0x20;
        RTI = 0x40;
        RTS = 0x60;

        PHP = 0x08;
        PLP = 0x28;
        PHA = 0x48;
        PLA = 0x68;

        DEY = 0x88;
        DEX = 0xca;
        TAY = 0xa8;
        INY = 0xc8;
        INX = 0xe8;

        CLC = 0x18;
        SEC = 0x38;
        CLI = 0x58;
        SEI = 0x78;
        TYA = 0x98;
        CLV = 0xb8;
        CLD = 0xd8;
        SED = 0xf8;

        TXA = 0x8a;
        TXS = 0x9a;
        TAX = 0xaa;
        TSX = 0xba;
    };
};

#endif // CPUOPCODES_H_INCLUDED
