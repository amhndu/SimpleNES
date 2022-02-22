#include "CPU.h"
#include "CPUOpcodes.h"
#include "Log.h"
#include <iomanip>

namespace sn
{
    CPU::CPU(MainBus &mem) :
        m_bus(mem)
    {}

    void CPU::reset()
    {
        reset(readAddress(ResetVector));
    }

    void CPU::reset(Address start_addr)
    {
        m_skipCycles = m_cycles = 0;
        r_A = r_X = r_Y = 0;
        f_I = true;
        f_C = f_D = f_N = f_V = f_Z = false;
        r_PC = start_addr;
        r_SP = 0xfd; //documented startup state
   }

    void CPU::interrupt(InterruptType type)
    {
        if (f_I && type != NMI && type != BRK_)
            return;

        if (type == BRK_) //Add one if BRK, a quirk of 6502
            ++r_PC;

        pushStack(r_PC >> 8);
        pushStack(r_PC);

        Byte flags = f_N << 7 |
                     f_V << 6 |
                       1 << 5 | //unused bit, supposed to be always 1
          (type == BRK_) << 4 | //B flag set if BRK
                     f_D << 3 |
                     f_I << 2 |
                     f_Z << 1 |
                     f_C;
        pushStack(flags);

        f_I = true;

        switch (type)
        {
            case IRQ:
            case BRK_:
                r_PC = readAddress(IRQVector);
                break;
            case NMI:
                r_PC = readAddress(NMIVector);
                break;
        }

        m_skipCycles += 7;
    }

    void CPU::pushStack(Byte value)
    {
        m_bus.write(0x100 | r_SP, value);
        --r_SP; //Hardware stacks grow downward!
    }

    Byte CPU::pullStack()
    {
        return m_bus.read(0x100 | ++r_SP);
    }

    void CPU::setZN(Byte value)
    {
        f_Z = !value;
        f_N = value & 0x80;
    }

    void CPU::setPageCrossed(Address a, Address b, int inc)
    {
        //Page is determined by the high byte
        if ((a & 0xff00) != (b & 0xff00))
            m_skipCycles += inc;
    }

    void CPU::skipDMACycles()
    {
        m_skipCycles += 513; //256 read + 256 write + 1 dummy read
        m_skipCycles += (m_cycles & 1); //+1 if on odd cycle
    }

    void CPU::step()
    {
        ++m_cycles;

        if (m_skipCycles-- > 1)
            return;

        m_skipCycles = 0;

        int psw =    f_N << 7 |
                     f_V << 6 |
                       1 << 5 |
                     f_D << 3 |
                     f_I << 2 |
                     f_Z << 1 |
                     f_C;
        LOG_CPU << std::hex << std::setfill('0') << std::uppercase
                  << std::setw(4) << +r_PC
                  << "  "
                  << std::setw(2) << +m_bus.read(r_PC)
                  << "  "
                  << "A:"   << std::setw(2) << +r_A << " "
                  << "X:"   << std::setw(2) << +r_X << " "
                  << "Y:"   << std::setw(2) << +r_Y << " "
                  << "P:"   << std::setw(2) << psw << " "
                  << "SP:"  << std::setw(2) << +r_SP  << /*std::endl;*/" "
                  << "CYC:" << std::setw(3) << std::setfill(' ') << std::dec << ((m_cycles - 1) * 3) % 341
                  << std::endl;

        Byte opcode = m_bus.read(r_PC++);

        auto CycleLength = OperationCycles[opcode];

        //Using short-circuit evaluation, call the other function only if the first failed
        //ExecuteImplied must be called first and ExecuteBranch must be before ExecuteType0
        if (CycleLength && (executeImplied(opcode) || executeBranch(opcode) ||
                        executeType1(opcode) || executeType2(opcode) || executeType0(opcode)))
        {
            m_skipCycles += CycleLength;
            //m_cycles %= 340; //compatibility with Nintendulator log
            //m_skipCycles = 0; //for TESTING
        }
        else
        {
            LOG(Error) << "Unrecognized opcode: " << std::hex << +opcode << std::endl;
        }
    }

    bool CPU::executeImplied(Byte opcode)
    {
        switch (static_cast<OperationImplied>(opcode))
        {
            case NOP:
                break;
            case BRK:
                interrupt(BRK_);
                break;
            case JSR:
                //Push address of next instruction - 1, thus r_PC + 1 instead of r_PC + 2
                //since r_PC and r_PC + 1 are address of subroutine
                pushStack(static_cast<Byte>((r_PC + 1) >> 8));
                pushStack(static_cast<Byte>(r_PC + 1));
                r_PC = readAddress(r_PC);
                break;
            case RTS:
                r_PC = pullStack();
                r_PC |= pullStack() << 8;
                ++r_PC;
                break;
            case RTI:
                {
                    Byte flags = pullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_D = flags & 0x8;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                r_PC = pullStack();
                r_PC |= pullStack() << 8;
                break;
            case JMP:
                r_PC = readAddress(r_PC);
                break;
            case JMPI:
                {
                    Address location = readAddress(r_PC);
                    //6502 has a bug such that the when the vector of anindirect address begins at the last byte of a page,
                    //the second byte is fetched from the beginning of that page rather than the beginning of the next
                    //Recreating here:
                    Address Page = location & 0xff00;
                    r_PC = m_bus.read(location) |
                           m_bus.read(Page | ((location + 1) & 0xff)) << 8;
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
                    pushStack(flags);
                }
                break;
            case PLP:
                {
                    Byte flags = pullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_D = flags & 0x8;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                break;
            case PHA:
                pushStack(r_A);
                break;
            case PLA:
                r_A = pullStack();
                setZN(r_A);
                break;
            case DEY:
                --r_Y;
                setZN(r_Y);
                break;
            case DEX:
                --r_X;
                setZN(r_X);
                break;
            case TAY:
                r_Y = r_A;
                setZN(r_Y);
                break;
            case INY:
                ++r_Y;
                setZN(r_Y);
                break;
            case INX:
                ++r_X;
                setZN(r_X);
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
                setZN(r_A);
                break;
            case CLV:
                f_V = false;
                break;
            case TXA:
                r_A = r_X;
                setZN(r_A);
                break;
            case TXS:
                r_SP = r_X;
                break;
            case TAX:
                r_X = r_A;
                setZN(r_X);
                break;
            case TSX:
                r_X = r_SP;
                setZN(r_X);
                break;
            default:
                return false;
        };
        return true;
    }

    bool CPU::executeBranch(Byte opcode)
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
                int8_t offset = m_bus.read(r_PC++);
                ++m_skipCycles;
                auto newPC = static_cast<Address>(r_PC + offset);
                setPageCrossed(r_PC, newPC, 2);
                r_PC = newPC;
            }
            else
                ++r_PC;
            return true;
        }
        return false;
    }

    bool CPU::executeType1(Byte opcode)
    {
        if ((opcode & InstructionModeMask) == 0x1)
        {
            Address location = 0; //Location of the operand, could be in RAM
            auto op = static_cast<Operation1>((opcode & OperationMask) >> OperationShift);
            switch (static_cast<AddrMode1>(
                    (opcode & AddrModeMask) >> AddrModeShift))
            {
                case IndexedIndirectX:
                    {
                        Byte zero_addr = r_X + m_bus.read(r_PC++);
                        //Addresses wrap in zero page mode, thus pass through a mask
                        location = m_bus.read(zero_addr & 0xff) | m_bus.read((zero_addr + 1) & 0xff) << 8;
                    }
                    break;
                case ZeroPage:
                    location = m_bus.read(r_PC++);
                    break;
                case Immediate:
                    location = r_PC++;
                    break;
                case Absolute:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    break;
                case IndirectY:
                    {
                        Byte zero_addr = m_bus.read(r_PC++);
                        location = m_bus.read(zero_addr & 0xff) | m_bus.read((zero_addr + 1) & 0xff) << 8;
                        if (op != STA)
                            setPageCrossed(location, location + r_Y);
                        location += r_Y;
                    }
                    break;
                case IndexedX:
                    // Address wraps around in the zero page
                    location = (m_bus.read(r_PC++) + r_X) & 0xff;
                    break;
                case AbsoluteY:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    if (op != STA)
                        setPageCrossed(location, location + r_Y);
                    location += r_Y;
                    break;
                case AbsoluteX:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    if (op != STA)
                        setPageCrossed(location, location + r_X);
                    location += r_X;
                    break;
                default:
                    return false;
            }

            switch (op)
            {
                case ORA:
                    r_A |= m_bus.read(location);
                    setZN(r_A);
                    break;
                case AND:
                    r_A &= m_bus.read(location);
                    setZN(r_A);
                    break;
                case EOR:
                    r_A ^= m_bus.read(location);
                    setZN(r_A);
                    break;
                case ADC:
                    {
                        Byte operand = m_bus.read(location);
                        std::uint16_t sum = r_A + operand + f_C;
                        //Carry forward or UNSIGNED overflow
                        f_C = sum & 0x100;
                        //SIGNED overflow, would only happen if the sign of sum is
                        //different from BOTH the operands
                        f_V = (r_A ^ sum) & (operand ^ sum) & 0x80;
                        r_A = static_cast<Byte>(sum);
                        setZN(r_A);
                    }
                    break;
                case STA:
                    m_bus.write(location, r_A);
                    break;
                case LDA:
                    r_A = m_bus.read(location);
                    setZN(r_A);
                    break;
                case SBC:
                    {
                        //High carry means "no borrow", thus negate and subtract
                        std::uint16_t subtrahend = m_bus.read(location),
                                 diff = r_A - subtrahend - !f_C;
                        //if the ninth bit is 1, the resulting number is negative => borrow => low carry
                        f_C = !(diff & 0x100);
                        //Same as ADC, except instead of the subtrahend,
                        //substitute with it's one complement
                        f_V = (r_A ^ diff) & (~subtrahend ^ diff) & 0x80;
                        r_A = diff;
                        setZN(diff);
                    }
                    break;
                case CMP:
                    {
                        std::uint16_t diff = r_A - m_bus.read(location);
                        f_C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
        return false;
    }

    bool CPU::executeType2(Byte opcode)
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
                    location = m_bus.read(r_PC++);
                    break;
                case Accumulator:
                    break;
                case Absolute_:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    break;
                case Indexed:
                    {
                        location = m_bus.read(r_PC++);
                        Byte index;
                        if (op == LDX || op == STX)
                            index = r_Y;
                        else
                            index = r_X;
                        //The mask wraps address around zero page
                        location = (location + index) & 0xff;
                    }
                    break;
                case AbsoluteIndexed:
                    {
                        location = readAddress(r_PC);
                        r_PC += 2;
                        Byte index;
                        if (op == LDX || op == STX)
                            index = r_Y;
                        else
                            index = r_X;
                        setPageCrossed(location, location + index);
                        location += index;
                    }
                    break;
                default:
                    return false;
            }

            std::uint16_t operand = 0;
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
                        setZN(r_A);
                    }
                    else
                    {
                        auto prev_C = f_C;
                        operand = m_bus.read(location);
                        f_C = operand & 0x80;
                        operand = operand << 1 | (prev_C && (op == ROL));
                        setZN(operand);
                        m_bus.write(location, operand);
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
                        setZN(r_A);
                    }
                    else
                    {
                        auto prev_C = f_C;
                        operand = m_bus.read(location);
                        f_C = operand & 1;
                        operand = operand >> 1 | (prev_C && (op == ROR)) << 7;
                        setZN(operand);
                        m_bus.write(location, operand);
                    }
                    break;
                case STX:
                    m_bus.write(location, r_X);
                    break;
                case LDX:
                    r_X = m_bus.read(location);
                    setZN(r_X);
                    break;
                case DEC:
                    {
                        auto tmp = m_bus.read(location) - 1;
                        setZN(tmp);
                        m_bus.write(location, tmp);
                    }
                    break;
                case INC:
                    {
                        auto tmp = m_bus.read(location) + 1;
                        setZN(tmp);
                        m_bus.write(location, tmp);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
        return false;
    }

    bool CPU::executeType0(Byte opcode)
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
                    location = m_bus.read(r_PC++);
                    break;
                case Absolute_:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    break;
                case Indexed:
                    // Address wraps around in the zero page
                    location = (m_bus.read(r_PC++) + r_X) & 0xff;
                    break;
                case AbsoluteIndexed:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    setPageCrossed(location, location + r_X);
                    location += r_X;
                    break;
                default:
                    return false;
            }
            std::uint16_t operand = 0;
            switch (static_cast<Operation0>((opcode & OperationMask) >> OperationShift))
            {
                case BIT:
                    operand = m_bus.read(location);
                    f_Z = !(r_A & operand);
                    f_V = operand & 0x40;
                    f_N = operand & 0x80;
                    break;
                case STY:
                    m_bus.write(location, r_Y);
                    break;
                case LDY:
                    r_Y = m_bus.read(location);
                    setZN(r_Y);
                    break;
                case CPY:
                    {
                        std::uint16_t diff = r_Y - m_bus.read(location);
                        f_C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                case CPX:
                    {
                        std::uint16_t diff = r_X - m_bus.read(location);
                        f_C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                default:
                    return false;
            }

            return true;
        }
        return false;
    }

    Address CPU::readAddress(Address addr)
    {
        return m_bus.read(addr) | m_bus.read(addr + 1) << 8;
    }

};

