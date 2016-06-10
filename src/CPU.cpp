#include "CPU.h"
#include "CPUOpcodes.h"
#include <iostream>
#include <iomanip>

#define LINE_LOG {std::cout << __LINE__ << " was executed" << std::endl;}
#define PRINT_VAR(x) {std::cout << #x << ": " << x << std::endl;}
#define PRINT_BYTE(x) {std::cout << #x << ": " << int(x) << std::endl;}

namespace sn
{
    CPU::CPU(MainMemory &mem) :
        m_SkipCycles(0),
        m_Cycles(0),
        m_Memory(mem)
    {
        Reset();
    }

    CPU::CPU(MainMemory &mem, Address start_addr) :
        m_SkipCycles(0),
        m_Cycles(0),
        m_Memory(mem)
    {
        Reset(start_addr);
    }

    void CPU::Reset()
    {
        f_I = true;
        r_PC = ReadAddress(ResetVector);
    }

    void CPU::Reset(Address start_addr)
    {
        f_I = true;
        r_PC = start_addr;
        r_SP = 0xfd; //for TESTING only! REMOVE this later
        m_SkipCycles = 3; //for TESTING only! REMOVE this later
    }

    void CPU::Interrupt(InterruptType type)
    {
        if (f_I && type != NMI)
            return;

        if (type == BRK_) //Add one if BRK, a quirk of 6502
            ++r_PC;
        PushStack(r_PC >> 8);
        PushStack(r_PC);

        Byte flags = f_N << 7 |
                     f_V << 6 |
                       1 << 5 | //unused bit, supposed to be always 1
          (type == BRK_) << 4 | //B flag set if BRK
                     f_D << 3 |
                     f_I << 2 |
                     f_Z << 1 |
                     f_C;
        PushStack(flags);

        f_I = true;

        switch (type)
        {
            case IRQ:
            case BRK_:
                r_PC = ReadAddress(IRQVector);
                break;
            case NMI:
                r_PC = ReadAddress(NMIVector);
                break;
        }

        m_SkipCycles += 7;
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

    void CPU::SetPageCrossed(Address a, Address b, int inc)
    {
        //Page is determined by the high byte
        if ((a & 0xff00) != (b & 0xff00))
            m_SkipCycles += inc;
    }
    void CPU::Execute()
    {
//        if (m_SkipCycles-- > 0)
//            return;

        int psw =    f_N << 7 |
                     f_V << 6 |
                       1 << 5 |
                     f_D << 3 |
                     f_I << 2 |
                     f_Z << 1 |
                     f_C;
        std::cout << std::hex << std::setfill('0') << std::uppercase
                  << std::setw(4) << int(r_PC)
                  << "    "
                  << std::setw(2) << int(Read(r_PC))
                  << "    "
                  << "A:"   << std::setw(2) << int(r_A) << " "
                  << "X:"   << std::setw(2) << int(r_X) << " "
                  << "Y:"   << std::setw(2) << int(r_Y) << " "
                  << "P:"   << std::setw(2) << psw << " "
                  << "SP:"  << std::setw(2) << int(r_SP) /* << " "
                  << "CYC:" << std::setw(3) << std::setfill(' ') << std::dec << m_Cycles*/
                  << std::endl;
        Byte opcode = Read(r_PC++);

        auto CycleLength = OperationCycles[opcode];
        //Using short-circuit evaluation, call the other function only if the first failed
        //ExecuteImplied must be called first and ExecuteBranch must be before ExecuteType0
        if (CycleLength && (ExecuteImplied(opcode) || ExecuteBranch(opcode) ||
                        ExecuteType1(opcode) || ExecuteType2(opcode) || ExecuteType0(opcode)))
        {
            m_SkipCycles += CycleLength;
            m_Cycles += m_SkipCycles;
            m_Cycles %= 340; //compatibility with Nintendulator log
            m_SkipCycles = 0; //for testing only, remove later
        }
        else
        {
            std::cerr << "Unrecognized opcode: " << std::hex << int(opcode) << std::endl;
        }
    }

    bool CPU::ExecuteImplied(Byte opcode)
    {
        switch (static_cast<OperationImplied>(opcode))
        {
            case NOP:
                break;
            case BRK:
                Interrupt(BRK_);
                break;
            case JSR:
                //Push address of next instruction - 1, thus r_PC + 1 instead of r_PC + 2
                //since r_PC and r_PC + 1 are address of subroutine
                PushStack(static_cast<Byte>((r_PC + 1) >> 8));
                PushStack(static_cast<Byte>(r_PC + 1));
                r_PC = ReadAddress(r_PC);
                break;
            case RTS:
                r_PC = PullStack();
                r_PC |= PullStack() << 8;
                ++r_PC;
                break;
            case RTI:
                {
                    Byte flags = PullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_D = flags & 0x8;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                r_PC = PullStack();
                r_PC |= PullStack() << 8;
                break;
            case JMP:
                r_PC = ReadAddress(r_PC);
                break;
            case JMPI:
                {
                    Address location = ReadAddress(r_PC);
                    PRINT_VAR(location)
                    r_PC = ReadAddress(location);
                    PRINT_VAR(r_PC)
                }
                break;
            case PHP:
                {
                    Byte flags = f_N << 7 |
                                 f_V << 6 |
                                   1 << 5 | //supposed to always be 1
                                   1 << 4 | //PHP pushes with the B flag as 1, no matter what
                                 f_D << 3 |
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
                    f_D = flags & 0x8;
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
            case CLD:
                f_D = false;
                break;
            case SED:
                f_D = true;
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
        if ((opcode & BranchInstructionMask) == BranchInstructionMaskResult)
        {
            //branch is initialized to the condition required (for the flag specified later)
            bool branch = opcode & BranchConditionMask;

            //set branch to true if the given condition is met by the given flag
            //We use xnor here, it is true if either both operands are true or false
            switch (opcode >> BranchOnFlagShift)
            {
                case Negative:
                    branch = !(branch ^ f_N);
                    break;
                case Overflow:
                    branch = !(branch ^ f_V);
                    break;
                case Carry:
                    branch = !(branch ^ f_C);
                    break;
                case Zero:
                    branch = !(branch ^ f_Z);
                    break;
                default:
                    return false;
            }

            if (branch)
            {
                Byte offset = Read(r_PC++);
                ++m_SkipCycles;

                SetPageCrossed(r_PC, r_PC + offset, 2);
                r_PC += offset;
            }
            else
                ++r_PC;
            return true;
        }
        return false;
    }

    bool CPU::ExecuteType1(Byte opcode)
    {
        if ((opcode & InstructionModeMask) == 0x1)
        {
            Address location = 0;
            switch (static_cast<AddrMode1>(
                    (opcode & AddrModeMask) >> AddrModeShift))
            {
                case IndexedIndirectX:
                    {
                        Byte zero_addr = r_X + Read(r_PC++);
                        //Addresses wrap in zero page mode, thus pass through a mask
                        location = Read(zero_addr & 0xff) | Read((zero_addr + 1) & 0xff) << 8;
                    }
                    break;
                case ZeroPage:
                    location = Read(r_PC++);
                    break;
                case Immediate:
                    location = r_PC++;
                    break;
                case Absolute:
                    location = ReadAddress(r_PC);
                    r_PC += 2;
                    break;
                case IndirectY:
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = Read(zero_addr & 0xff) | Read((zero_addr + 1) & 0xff) << 8;
                        SetPageCrossed(location, location + r_Y);
                        location += r_Y;
                    }
                    break;
                case IndexedX:
                    location = Read(r_PC++) + r_X;
                    break;
                case AbsoluteY:
                    {
                        Address addr = ReadAddress(r_PC);
                        r_PC += 2;
                        location = ReadAddress(addr);
                        SetPageCrossed(location, location + r_Y);
                        location += r_Y;
                    }
                    break;
                case AbsoluteX:
                    {
                        Address addr = ReadAddress(r_PC);
                        r_PC += 2;
                        location = ReadAddress(addr);
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
                        //Carry forward or UNSIGNED overflow
                        f_C = sum & 0x100;
                        //SIGNED overflow, would only happen if the sign of sum is
                        //different from BOTH the operands
                        f_V = (r_A ^ sum) & (operand ^ sum) & 0x80;
                        r_A = static_cast<Byte>(sum);
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
                case SBC:
                    {
                        //High carry means "no borrow", thus negate and subtract
                        uint16_t subtrahend = Read(location),
                                 diff = r_A - subtrahend - !f_C;
                        //if the ninth bit is 1, the resulting number is negative => borrow => low carry
                        f_C = !(diff & 0x100);
                        //Same as ADC, except instead of the subtrahend,
                        //substitute with it's one complement
                        f_V = (r_A ^ diff) & (~subtrahend ^ diff) & 0x80;
                        r_A = diff;
                        SetZN(diff);
                    }
                    break;
                case CMP:
                    {
                        uint16_t diff = r_A - Read(location);
                        f_C = !(diff & 0x100);
                        SetZN(diff);
                    }
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
        if ((opcode & InstructionModeMask) == 2)
        {
            Address location = 0;
            auto op = static_cast<Operation2>((opcode & OperationMask) >> OperationShift);
            auto addr_mode =
                    static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift);
            switch (addr_mode)
            {
                case Immediate_:
                    location = r_PC++;
                    break;
                case ZeroPage_:
                    location = Read(r_PC++);
                    break;
                case Accumulator:
                    break;
                case Absolute_:
                    location = ReadAddress(r_PC);
                    r_PC += 2;
                    break;
                case Indexed:
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = ReadAddress(zero_addr);
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
                        Address addr = ReadAddress(r_PC);
                        r_PC += 2;
                        location = ReadAddress(addr);
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
                    if (addr_mode == Accumulator)
                    {
                        auto prev_C = f_C;
                        f_C = r_A & 0x80;
                        r_A <<= 1;
                        //If Rotating, set the bit-0 to the the previous carry
                        r_A = r_A | (prev_C && (op == ROL));
                        SetZN(r_A);
                    }
                    else
                    {
                        auto prev_C = f_C;
                        operand = Read(location);
                        f_C = operand & 0x80;
                        operand = operand << 1 | (prev_C && (op == ROL));
                        SetZN(operand);
                        Write(location, operand);
                    }
                    break;
                case LSR:
                case ROR:
                    if (addr_mode == Accumulator)
                    {
                        auto prev_C = f_C;
                        f_C = r_A & 1;
                        r_A >>= 1;
                        //If Rotating, set the bit-7 to the previous carry
                        r_A = r_A | (prev_C && (op == ROR)) << 7;
                        SetZN(r_A);
                    }
                    else
                    {
                        auto prev_C = f_C;
                        operand = Read(location);
                        f_C = operand & 1;
                        operand = operand >> 1 | (prev_C && (op == ROR)) << 7;
                        SetZN(operand);
                        Write(location, operand);
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
                        auto tmp = Read(location) - 1;
                        SetZN(tmp);
                        Write(location, tmp);
                    }
                    break;
                case INC:
                    {
                        auto tmp = Read(location) + 1;
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
        if ((opcode & InstructionModeMask) == 0x0)
        {
            Address location = 0;
            switch (static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift))
            {
                case Immediate_:
                    location = r_PC++;
                    break;
                case ZeroPage_:
                    location = Read(r_PC++);
                    break;
                case Absolute_:
                    location = ReadAddress(r_PC);
                    r_PC += 2;
                    break;
                case Indexed:
                    {
                        Byte zero_addr = Read(r_PC++);
                        location = ReadAddress(zero_addr);
                        SetPageCrossed(location, location + r_X);
                        location += r_X;
                    }
                    break;
                case AbsoluteIndexed:
                    {
                        Address addr = ReadAddress(r_PC);
                        r_PC += 2;
                        location = ReadAddress(addr);
                        SetPageCrossed(location, location + r_X);
                        location += r_X;
                    }
                    break;
                default:
                    return false;
            }
            uint16_t operand = 0;
            switch (static_cast<Operation0>((opcode & OperationMask) >> OperationShift))
            {
                case BIT:
                    operand = Read(location);
                    f_Z = !(r_A & operand);
                    f_V = operand & 0x40;
                    f_N = operand & 0x80;
                    break;
                case STY:
                    Write(location, r_Y);
                    break;
                case LDY:
                    r_Y = Read(location);
                    SetZN(r_Y);
                    break;
                case CPY:
                    {
                        uint16_t diff = r_Y - Read(location);
                        f_C = !(diff & 0x100);
                        SetZN(diff);
                    }
                    break;
                case CPX:
                    {
                        uint16_t diff = r_X - Read(location);
                        f_C = !(diff & 0x100);
                        SetZN(diff);
                    }
                    break;
                default:
                    return false;
            }

            return true;
        }
        return false;
    }

    Byte CPU::Read(Address addr)
    {
        return m_Memory[addr];
    }

    Address CPU::ReadAddress(Address addr)
    {
        return m_Memory[addr] | m_Memory[addr + 1] << 8;
    }

    void CPU::Write(Address addr, Byte value)
    {
        m_Memory[addr] = value;
    }
};
