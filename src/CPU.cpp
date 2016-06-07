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

    void CPU::SubtractAndSet(Byte a, Byte b)
    {
        //High carry means "no borrow"
        uint16_t diff = a - b - !f_C,
        f_C = diff & 0x100;
        //A formula obtained by analyzing the different cases of signs of operands and the result
        f_V = (a ^ diff) & (a ^ b) & 0x80;
        SetZN(diff);
        return diff;

    }

    void CPU::SetPageCrossed(Address a, Address b, int inc)
    {
        if ((a & 0xff00) != (b & 0xff00))
            m_SkipCycles += inc;
    }
    void CPU::Execute()
    {
        if (m_SkipCycles-- > 0)
            return;

        Byte opcode = Read(r_PC);

        auto CycleLength = OperationCycles[opcode];
        //Using short-circuit evaluation, call the other function only if the first failed
        //ExecuteImplied must be called first and ExecuteBranch must be before ExecuteType0
        if (CycleLength && (ExecuteImplied() || ExecuteBranch() ||
                        ExecuteType1() || ExecuteType2() || ExecuteType0()))
        {
            m_SkipCycles += length;
        }
        else
        {
            std::cerr << "Unrecognized opcode: " << std::hex << opcode << std::endl;
        }

        ++r_PC;
    }

    bool CPU::ExecuteImplied(Byte opcode)
    {
        switch (static_cast<OperationImplied>(opcode))
        {
            case NOP:
                break;
            case BRK:
                f_B = true;
                ++r_PC;
                TODO;
                break;
            case JSR:
                PushStack(static_cast<Byte>(r_PC >> 8));
                PushStack(static_cast<Byte>(r_PC + 2));
                r_PC = Read(r_PC + 1) | Read(r_PC + 2) << 8;
                break;
            case RTS:
                r_PC = PullStack() + 1;
                r_PC |= PullStack() << 8;
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
                break;
            case JMP:
                r_PC = Read(r_PC + 1) | Read(r_PC + 2) << 8;
                break;
            case JMPI:
                {
                    Address location = Read(r_PC + 1) | Read(r_PC + 2) << 8;
                    r_PC = Read(location) | Read(location + 1) << 8;
                }
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
                break;
            case PHA:
                PushStack(r_A);
                break;
            case PLA:
                r_A = PullStack();
                SetZN(r_A);
                break;
            case DEY:
                --r_Y;
                SetZN(r_Y);
                break;
            case DEX:
                --r_X;
                SetZN(r_X);
                break;
            case TAY:
                r_Y = r_A;
                SetZN(r_Y);
                break;
            case INY:
                ++r_Y;
                SetZN(r_Y);
                break;
            case INX:
                ++r_X;
                SetZN(r_X);
                break;
            case CLC:
                f_C = false;
                break;
            case SEC:
                f_C = true;
                break;
            case CLI:
                f_I = false;
                break;
            case SEI:
                f_I = true;
                break;
            case TYA:
                r_A = r_Y;
                SetZN(r_A);
                break;
            case CLV:
                f_V = false;
                break;
            case TXA:
                r_A = r_X;
                SetZN(r_A);
                break;
            case TXS:
                r_SP = r_X;
                break;
            case TAX:
                r_X = r_A;
                SetZN(r_X);
                break;
            case TSX:
                r_X = r_SP;
                SetZN(r_X);
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
            if (branch)
            {
                Byte offset = Read(r_PC);
                ++m_SkipCycles;

                SetPageCrossed(r_PC, r_PC + offset, 2);
                r_PC += offset;
            }
            return true;
        }
        return false;
    }

    bool CPU::ExecuteType1(Byte opcode)
    {
        if (opcode & InstructionModeMask == 0x1)
        {
            Address location = 0;
            switch (static_cast<AddrMode1>(AddrMode1 addr_mode =
                                    (opcode & AddrModeMask) >> AddrModeShift))
            {
                case IndexedIndirectX:
                    {
                        Address addr = Read(r_X + Read(r_PC++));
                        location = Read(addr) | Read(addr + 1) << 8;
                    }
                    break;
                case ZeroPage:
                    location = Read(r_PC++);
                    break;
                case Immediate:
                    location = r_PC++;
                    break;
                case Absolute:
                    location = Read(r_PC++);
                    location = location | Read(r_PC++) << 8;
                    break;
                case IndirectY:
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = Read(zero_addr) | Read(zero_addr + 1) << 8;
                        SetPageCrossed(location, location + r_Y);
                        location += r_Y;
                    }
                    break;
                case IndexedX;
                    location = Read(r_PC++) + r_X;
                    break;
                case AbsoluteY:
                    {
                        Address addr = Read(r_PC++);
                        addr = addr | Read(r_PC++) << 8;
                        location = Read(addr) | Read(addr + 1) << 8;
                        SetPageCrossed(location, location + r_Y);
                        location += r_Y;
                    }
                    break;
                case AbsoluteX:
                    {
                        Address addr = Read(r_PC++);
                        addr = addr | Read(r_PC++) << 8;
                        location = Read(addr) | Read(addr + 1) << 8;
                        SetPageCrossed(location, location + r_X);
                        location += r_X;
                    }
                    break;
                default:
                    return false;
            }

            switch (static_cast<Operation1>(
                                (opcode & OperationMask) >> OperationShift))
            {
                case ORA:
                    r_A |= Read(location);
                    SetZN(r_A);
                    break;
                case AND:
                    r_A &= Read(location);
                    SetZN(r_A);
                    break;
                case EOR:
                    r_A ^= Read(location);
                    SetZN(r_A);
                    break;
                case ADC:
                    {
                        Byte operand = Read(location);
                        uint16_t sum = r_A + operand + f_C;
                        r_A = static_cast<Byte>(sum);
                        f_C = sum & 0x100;
                        f_V = (r_A ^ sum) & (r_A ^ operand) & 0x80;
                        SetZN(r_A);
                    }
                    break;
                case STA:
                    Write(location, r_A);
                    break;
                case LDA:
                    r_A = Read(location);
                    SetZN(r_A);
                    break;
                case SDC:
                    r_A = SubtractAndSet(r_A, Read(location));
                    break;
                case CMP:
                    SubtractAndSet(r_A, Read(location));
                    break;
                default:
                    return false;
            }
            return true;
        }
        return false;
    }

    bool CPU::ExecuteType2(Byte opcode)
    {
        if (opcode & InstructionModeMask == 0x2)
        {
            Address location = 0;
            Operation2 op = (opcode & OperationMask) >> OperationShift;
            switch (static_cast<AddrMode2>(AddrMode2 addr_mode =
                                    (opcode & AddrModeMask) >> AddrModeShift))
            {
                case Immediate:
//                    if (op != LDX)
//                        return false;
                    location = r_PC++;
                    break;
                case ZeroPage:
                    location = Read(r_PC++);
                    break;
                case Accumulator:
//                    switch (op)
//                    {
//                        case STX:
//                        case LDX:
//                        case DEC:
//                        case INC:
//                            return false:
//                        default:
//                            break;
//                    }
                    break;
                case Absolute:
                    location = Read(r_PC++);
                    location = location | Read(r_PC++) << 8;
                    break;
                case Indexed:
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = Read(zero_addr) | Read(zero_addr + 1) << 8;
                        Byte index;
                        if (op == LDX || op == STX)
                            index = r_Y;
                        else
                            index = r_X;
                        SetPageCrossed(location, location + index);
                        location += index;
                    }
                    break;
                case AbsoluteIndexed:
                    {
                        Address addr = Read(r_PC++);
                        addr = addr | Read(r_PC++) << 8;
                        location = Read(addr);
                        Byte index;
                        if (op == LDX || op == STX)
                            index = r_Y;
                        else
                            index = r_X;
                        SetPageCrossed(location, location + index);
                        location += index;
                    }
                    break;
                default:
                    return false;
            }

            uint16_t operand = 0;
            switch (op)
            {
                case ASL:
                case ROL:
                    if (mode == Accumulator)
                    {
                        f_C = r_A & 0x100;
                        r_A <<= 1;
                        SetZN(r_A);
                        r_A = r_A | (f_C & op == ROL);
                    }
                    else
                    {
                        operand = Read(location);
                        f_C = operand & 0x100;
                        operand <<= 1;
                        SetZN(operand);
                        Write(location, operand | (f_C & op == ROL));
                    }
                    break;
                case LSR:
                case ROR:
                    if (mode == Accumulator)
                    {
                        f_C = r_A & 1;
                        r_A >>= 1;
                        SetZN(r_A);
                        r_A = r_A | (f_C & op == ROR) << 7;
                    }
                    else
                    {
                        operand = Read(location);
                        f_C = operand & 1;
                        operand >>= 1;
                        SetZN(operand);
                        Write(location, operand | (f_C & op == ROR) << 7);
                    }
                    break;
                case STX:
                    Write(location, r_X);
                    break;
                case LDX:
                    r_X = Read(location);
                    SetZN(r_X);
                    break;
                case DEC:
                    {
                        auto tmp = --Read(location);
                        SetZN(tmp);
                        Write(location, tmp);
                    }
                    break;
                case INC:
                    {
                        auto tmp = ++Read(location);
                        SetZN(tmp);
                        Write(location, tmp);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
        return false;
    }

    bool CPU::ExecuteType0(Byte opcode)
    {
        if (opcode & InstructionModeMask == 0x0)
        {
            Operation0 op = (opcode & OperationMask) >> OperationShift;
            Address location = 0;
            switch (static_cast<AddrMode2>(AddrMode0 addr_mode =
                                (opcode & AddrModeMask) >> AddrModeShift));
            {
                case Immediate:
                    location = r_PC++;
                    break;
                case ZeroPage:
                    location = Read(r_PC++);
                    break;
                case Absolute:
                    location = Read(r_PC++);
                    location = location | Read(r_PC++) << 8;
                    break;
                case Indexed:
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = Read(zero_addr) | Read(zero_addr + 1) << 8;
                        SetPageCrossed(location, location + r_X);
                        location += r_X;
                    }
                    break;
                case AbsoluteIndexed:
                    {
                        Address addr = Read(r_PC++);
                        addr = addr | Read(r_PC++) << 8;
                        location = Read(addr);
                        SetPageCrossed(location, location + r_X);
                        location += r_X;
                    }
                    break;
                default:
                    return false;
            }
            uint16_t operand = 0;
            switch (op)
            {
                case BIT:
                    operand = Read(location);
                    f_Z = r_A & operand;
                    f_V = operand & 0x40;
                    f_C = operand & 0x80;
                    break;
                case STY:
                    Write(location, r_Y);
                    break;
                case LDY:
                    r_Y = Read(location);
                    SetZN(r_Y);
                    break;
                case CPY:
                    SubtractAndSet(r_Y, Read(location));
                    break;
                case CPX:
                    SubtractAndSet(r_X, Read(location));
                    break;
                default:
                    return false;
            }

            return true
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
