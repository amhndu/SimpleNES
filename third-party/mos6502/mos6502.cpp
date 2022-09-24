#include "mos6502.hpp"
#include "bit.hpp"

#define A register_.a
#define X register_.x
#define Y register_.y
#define S register_.s
#define P register_.p
#define PC register_.pc.w
#define PCH register_.pc.hi
#define PCL register_.pc.lo
#define MAR register_.mar.w
#define MDR register_.mdr
#define C register_.p.c
#define Z register_.p.z
#define I register_.p.i
#define D register_.p.d
#define B register_.p.b
#define U register_.p.u
#define V register_.p.v
#define N register_.p.n
#define ADDRESS (this->*addr)
#define ALGORITHM (this->*alg)

namespace sen {
auto MOS6502::power() -> void {
  A = 0x00;
  X = 0x00;
  Y = 0x00;
  S = 0xff;
  P = 0x04;
  MDR = 0x00;
}

auto MOS6502::mdr() const -> uint8_t {
  return MDR;
}

auto MOS6502::interrupt(uint16_t vector) -> void {
  r16 origin_pc{};
  idleRead();
  idleRead();
  origin_pc.w = PC;

  push(origin_pc.hi);
  push(origin_pc.lo);
  nmi(vector);
  push(P | 0x20);   // irq and nmi just set U to 1, brk also set B to 1
  I = 1;
  PCL = read(vector);
  PCH = read(vector + 1);
}
}

#include "addressing.icc"
#include "algorithm.icc"
#include "instruction.icc"
#include "memory.icc"

#undef ALGORITHM
#undef ADDRESS
#undef N
#undef V
#undef U
#undef B
#undef D
#undef I
#undef Z
#undef C
#undef MDR
#undef MAR
#undef PCL
#undef PCH
#undef PC
#undef P
#undef S
#undef Y
#undef X
#undef A