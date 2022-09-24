#pragma once
#ifndef MOS6502_HPP_
#define MOS6502_HPP_

#include "reg-type.hpp"

namespace sen {

struct MOS6502 {
  union Flags {
    r8 byte;
    EmbedBitRange<0> c;     // carry
    EmbedBitRange<1> z;     // zero
    EmbedBitRange<2> i;     // interrupt disable
    EmbedBitRange<3> d;     // decimal mode
    EmbedBitRange<4> b;     // break
    EmbedBitRange<5> u;     // unused
    EmbedBitRange<6> v;     // overflow
    EmbedBitRange<7> n;     // negative

    inline operator r8() const { return byte; } // NOLINT(google-explicit-constructor)
    inline auto operator=(r8 data) -> Flags & {
      byte &= 0b00110000;
      data &= 0b11001111;
      byte |= data;
      return *this;
    }
  };
  struct Register {
    r8 a, x, y, s;
    r16 pc;
    Flags p;

    r8 mdr;
    r16 mar;
  };
  static_assert(sizeof(Register) == 10, "Register size error");

  virtual auto read(uint16_t) -> uint8_t = 0;
  virtual auto write(uint16_t, uint8_t) -> void = 0;
  virtual auto nmi(uint16_t& vector) -> void = 0;
  virtual auto cancelNmi() -> void = 0;
  virtual auto delayIrq() -> void = 0;

  // @file: mos6502.cpp
  auto mdr() const -> uint8_t;
  auto power() -> void;
  auto interrupt(uint16_t vector) -> void;

  // @file: memory.icc
  auto opcode() -> uint8_t;
  auto operand() -> uint8_t;
  auto load(uint8_t addr) -> uint8_t;
  auto store(uint8_t addr, uint8_t data) -> void;
  auto push(uint8_t data) -> void;
  auto pull() -> uint8_t;

  auto idleRead() -> void;
  auto idleZeroPage(uint8_t addr) -> void;
  auto idlePageCrossed(uint16_t x, uint16_t y, bool isWrite) -> void;
  auto idleStackPointer() -> void;

  // @file: addressing.icc
  using Addressing = auto (MOS6502::*)()->uint16_t;
  enum class AddressingCode {
    Absolute,
    AbsoluteXRead,
    AbsoluteXWrite,
    AbsoluteYRead,
    AbsoluteYWrite,
    Accumulator,
    Immediate,
    Implied,
    Indirect,
    IndirectX,
    IndirectYRead,
    IndirectYWrite,
    Relative,
    ZeroPage,
    ZeroPageX,
    ZeroPageY,
  };

  auto absoluteIndexed(uint8_t index, bool is_write) -> uint16_t;
  auto indirectYIndexed(bool is_write) -> uint16_t;
  // default addressing is nothing to do
  template<AddressingCode mode> auto addressing() -> uint16_t { return 0; }

  // @file: algorithm.icc
  using Algorithm = auto (MOS6502::*)()->void;
  enum class AlgorithmCode {
    // official algorithm code
    ADC, AND, ASL_A, ASL_M, BCC, BCS, BEQ, BIT,
    BMI, BNE, BPL, BRK, BVC, BVS, CLC, CLD,
    CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY,
    EOR, INC, INX, INY, JMP, JSR, LDA, LDX,
    LDY, LSR_A, LSR_M, NOP, ORA, PHA, PHP, PLA,
    PLP, ROL_A, ROL_M, ROR_A, ROR_M, RTI, RTS, SBC,
    SEC, SED, SEI, STA, STX, STY, TAX, TAY,
    TSX, TXA, TXS, TYA,
    // unofficial algorithm code
    AAC, AXA, AAX, ARR, ASR, ATX, AXS, DCP,
    ISB, LAR, LAX, RLA, RRA, SLO, SRE, SXA,
    SYA, XAS, XAA, KIL,
    // Instructions effecting in 'no operations' in various address modes. Operands are ignored.
    DOP = NOP, TOP = NOP, USBC = SBC,
  };
  template<AlgorithmCode code> auto algorithm() -> void {}
  auto addMemoryWithCarry(uint8_t in) -> uint8_t;
  auto branchOnStatusFlag(bool take, uint8_t in) -> void;
  auto bitMemory(uint8_t in) -> void;
  auto compare(uint8_t reg, uint8_t in) -> uint8_t;
  auto loadMemory(uint8_t) -> uint8_t;
  auto shiftLeft(uint8_t) -> uint8_t;
  auto shiftRight(uint8_t) -> uint8_t;
  auto rotateLeft(uint8_t) -> uint8_t;
  auto rotateRight(uint8_t) -> uint8_t;
  auto subtract(uint8_t in) -> uint8_t;

  // @file: instruction.icc
  enum class InstructionCode { Load, Write, Modify, Jump };
  using Instruction = auto (MOS6502::*)(Addressing, Algorithm)->void;
  template<InstructionCode code> auto instruction(Addressing, Algorithm) -> void {}
  auto runInstruction() -> void;

 protected:
  Register register_;
};

}

#endif //MOS6502_HPP_
