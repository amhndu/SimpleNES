#include "CPU.h"

namespace sn
{
    CPU::CPU()
    {
    }

    CPU::~CPU()
    {
    }

    void CPU::PushStack(Byte value)
    {
        Write(0x100 | r_SP, value);
        --r_SP; //Hardware stacks grow downward!
    }

    Byte CPU::PullStack()
    {
        return Read(0x100 | ++r_SP);
    }

    void CPU::SetZN(Byte value)
    {
        f_Z = !value;
        f_N = value & 0x80;
    }

    void CPU::Execute()
    {
        if (m_SkipCycles-- > 0)
            return;

        Byte opcode = Read(r_PC);

        //Using short-circuit evaluation, call the other function only if the first failed
        if (! (ExecuteImplied() || ExecuteBranch() || ExecuteType01() || ExecuteType10() || ExecuteType00()) )
            std::cerr << "Unrecognized opcode: " << std::hex << opcode << std::endl;

        ++r_PC;
    }

    bool CPU::ExecuteImplied(Byte opcode)
    {
        switch (static_cast<OpcodeImplied>(opcode))
        {
            case NOP:
                break;
            case BRK:
                f_B = true;
                ++r_PC;
                m_SkipCycles = 7;
                TODO;
                break;
            case JSR:
                PushStack(static_cast<Byte>(r_PC >> 8));
                PushStack(static_cast<Byte>(r_PC + 2));
                r_PC = Read(r_PC + 1) | Read(r_PC + 2) << 8;
                m_SkipCycles = 6;
                break;
            case RTS:
                r_PC = PullStack() + 1;
                r_PC |= PullStack() << 8;
                m_SkipCycles = 6;
                break;
            case RTI:
                {
                    Byte flags = PullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_B = flags & 0x10;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                r_PC = PullStack() + 1;
                r_PC |= PullStack() << 8;
                m_SkipCycles = 6;
                break;
            case PHP:
                {
                    Byte flags = f_N << 7 |
                                 f_V << 6 |
                                 f_B << 4 |
                                 f_I << 2 |
                                 f_Z << 1 |
                                 f_C;
                    PushStack(flags);
                }
                m_SkipCycles = 3;
                break;
            case PLP:
                {
                    Byte flags = PullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_B = flags & 0x10;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                m_SkipCycles = 4;
                break;
            case PHA:
                PushStack(r_A);
                m_SkipCycles = 3;
                break;
            case PLA:
                r_A = PullStack();
                SetZN(r_A);
                m_SkipCycles = 4;
                break;
            case DEY:
                --r_Y;
                SetZN(r_Y);
                m_SkipCycles = 2;
                break;
            case DEX:
                --r_X;
                SetZN(r_X);
                m_SkipCycles = 2;
                break;
            case TAY:
                r_Y = r_A;
                SetZN(r_Y);
                m_SkipCycles = 2;
                break;
            case INY:
                ++r_Y;
                SetZN(r_Y);
                m_SkipCycles = 2;
                break;
            case INX:
                ++r_X;
                SetZN(r_X);
                m_SkipCycles = 2;
                break;
            case CLC:
                f_C = false;
                m_SkipCycles = 2;
                break;
            case SEC:
                f_C = true;
                m_SkipCycles = 2;
                break;
            case CLI:
                f_I = false;
                m_SkipCycles = 2;
                break;
            case SEI:
                f_I = true;
                m_SkipCycles = 2;
                break;
            case TYA:
                r_A = r_Y;
                SetZN(r_A);
                m_SkipCycles = 2;
                break;
            case CLV:
                f_V = false;
                m_SkipCycles = 2;
                break;
            case TXA:
                r_A = r_X;
                SetZN(r_A);
                m_SkipCycles = 2;
                break;
            case TXS:
                r_SP = r_X;
                m_SkipCycles = 2;
                break;
            case TAX:
                r_X = r_A;
                SetZN(r_X);
                m_SkipCycles = 2;
                break;
            case TSX:
                r_X = r_SP;
                SetZN(r_X);
                m_SkipCycles = 2;
                break;
            default:
                return false;
        };
        return true;
    }

    bool CPU::ExecuteBranch(Byte opcode)
    {
        if (opcode & 0x1f == 0x10)
        {
            ++r_PC;
            bool branch = opcode & 0x40;
            switch (opcode >> 6)
            {
                case 0x0:
                    branch = branch && f_N;
                    break;
                case 0x1:
                    branch = branch && f_V;
                    break;
                case 0x2:
                    branch = branch && f_C;
                    break;
                case 0x3:
                    branch = branch && f_Z;
                    break;
                default:
                    return false;
            }
            m_SkipCycles = 2;
            if (branch)
            {
                Byte offset = Read(r_PC);
                ++m_SkipCycles;

                //***TO VERIFY. This check for a change in page is probably not true.
                if ((r_PC % 0x100) != ((r_PC + offset) % 0x100))
                    m_SkipCycles += 2;
            }
            return true;
        }
        return false;
    }

    bool CPU::ExecuteType01(Byte opcode)
    {
        if (opcode & 0x3 == 01)
        {
            Address location = 0;
            Byte index = 0;
            switch (static_cast<Mode01>(Mode01 addr_mode = opcode & 0x1c))
            {
                case IndexedIndirectX:
                    {
                        Address addr = Read(r_X + Read(r_PC++));
                        location = Read(addr) | Read(addr + 1) << 8;
                        m_SkipCycles = 6;
                    }
                    break;
                case ZeroPage:
                    location = Read(r_PC++);
                    m_SkipCycles = 3;
                    break;
                case Immediate:
                    location = r_PC++;
                    m_SkipCycles = 2;
                    break;
                case Absolute:
                    location = Read(r_PC++);
                    location = location | Read(r_PC++) << 8;
                    m_SkipCycles = 4;
                    break;
                case IndirectY:
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = Read(zero_addr) | Read(zero_addr + 1) << 8;
                        index = r_Y;
                        m_SkipCycles = 5;
                    }
                    break;
                case IndexedX;
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = Read(zero_addr) | Read(zero_addr + 1) << 8;
                        index = r_X;
                        m_SkipCycles= 5;
                    }
                    break;
                case AbsoluteY:
                    {
                        Address addr = Read(r_PC++);
                        addr = addr | Read(r_PC++) << 8;
                        location = Read(addr);
                        index = r_Y;
                        m_SkipCycles = 4;
                    }
                    break;
                case AbsoluteX:
                    {
                        Address addr = Read(r_PC++);
                        addr = addr | Read(r_PC++) << 8;
                        location = Read(addr)
                        index = r_X;
                        m_SkipCycles = 4;
                    }
                    break;
                default:
                    return false;
            }

            switch (static_cast<Opcode01>(opcode & 0xe0))
            {
                case ORA:
                    r_A |= Read(location) + index;
                    SetZN(r_A);
                    break;
                case AND:
                    r_A &= Read(location) + index;
                    SetZN(r_A);
                    break;
                case EOR:
                    r_A ^= Read(location) + index;
                    SetZN(r_A);
                    break;
                case ADC:
                    {
                        Byte operand = Read(location) + index;
                        uint16_t sum = r_A + operand + (f_C) ? 1 : 0;
                        r_A = static_cast<Byte>(sum);
                        f_C = sum >> 8 & 1;
                        f_V = (r_A ^ sum) & (r_A ^ operand) & 0x80;
                        SetZN(r_A);
                    }
                    break;
                case STA:
                    if (addr_mode == Immediate)
                        return false;
                    else
                        Write(location, r_A);
                    break;
                case LDA:
                    r_A = Read(location);
                    SetZN(r_A);
                    break;
                case SDC:
                    Byte operand = Read(location) + index;
                    uint16_t diff = r_A - operand - (f_C) ? 0 : 1;
                    r_A = static_cast<Byte>(diff);
                    //No break!
                case CMP:
                    f_C = sum >> 8 & 1;
                    f_V = (r_A ^ sum) & (r_A ^ operand) & 0x80;
                    SetZN(r_A);
                    break;
                default:
                    std::cerr << "Invalid opcode/overflow" << std::endl;
            }
            return true;
        }
        return false;
    }

    bool CPU::ExecuteType10(Byte opcode)
    {
        if (opcode & 0x3 == 10)
        {
            Address location = 0;
            Byte index = 0;
            switch (static_cast<Mode10>(Mode10 addr_mode = opcode & 0x1c))
            {
                case Immediate:
                    location = r_PC++;
                    break;
                case ZeroPage:
                    location = Read(r_PC++);
                    break;
                case Accumulator:
                    break;
                case Absolute:
                    location = Read(r_PC++);
                    location = location | Read(r_PC++) << 8;
                    break;
                case Indexed:
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = Read(zero_addr) | Read(zero_addr + 1) << 8;
                    }
                    break;
                case AbsoluteIndexed:
                    {
                        Address addr = Read(r_PC++);
                        addr = addr | Read(r_PC++) << 8;
                        location = Read(addr)
                    }
                    break;
                default:
                    return false;
            }

            ADD SKIP CYCLES;

            uint16_t operand = 0;
            switch (static_cast<Opcode01>(opcode & 0xe0))
            {
                case ASL:
                    if (mode == Accumulator)
                    {
                        f_C = r_A >> 8 & 1;
                        r_A <<= 1;
                        SetZN(r_A);
                    }
                    else
                    {
                        if (mode == Indexed || mode == AbsoluteIndexed)
                            index == r_X;
                        operand = Read(location) + index;
                        f_C = operand >> 8 & 1;
                        operand <<= 1;
                        SetZN(operand);
                    }
                    break;
                case ROL:
                    if (mode == Accumulator)
                    {
                        f_C = r_A >> 8 & 1;
                        r_A = (r_A << 1) | f_C;
                        SetZN(r_A);
                    }
                    else
                    {
                        if (mode == Indexed || mode == AbsoluteIndexed)
                            index == r_X;
                        operand = Read(location) + index;
                        f_C = operand >> 8 & 1;
                        operand = (operand << 1) | f_C;
                        SetZN(operand);
                    }
                    break;
                case LSR:
                    break;
                case ROR:
                    break;
                case STX:
                    break;
                case LDX:
                    break;
                case DEC:
                    break;
                case INC:
                    break;
            }
            return true;
        }
        return false;
    }

    Byte CPU::Read(Address addr)
    {
        if (addr < 0x2000)
        {
            //Locations 0x0 - 0x7ff are mirrored three times up to 0x1fff
            return RAM[addr & 0x7ff];
        }
    }
    Byte CPU::ReadRAM(Address addr)
    {
        return RAM[addr & 0x7ff];
    }

    void CPU::Write(Address addr, Byte value)
    {
        if (addr < 0x2000)
        {
            RAM[addr & 0x7ff] = value;
        }
    }
    void CPU::WriteRAM(Address addr, Byte value)
    {
        RAM[addr & 0x7ff] = value;
    }
};
