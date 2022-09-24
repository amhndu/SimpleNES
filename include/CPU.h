#ifndef CPU_H
#define CPU_H
#include "CPUOpcodes.h"
#include "MainBus.h"
#include "mos6502.hpp"

namespace sn {

class CPU : public sen::MOS6502 {
 public:
  explicit CPU(MainBus &mem);

  auto read(uint16_t addr) -> uint8_t override {
    ++m_skipCycles;
    return m_bus.read(addr);
  }
  auto write(uint16_t addr, uint8_t data) -> void override {
    ++m_skipCycles;
    m_bus.write(addr, data);
  }
  auto nmi(uint16_t &vector) -> void override {}
  auto cancelNmi() -> void override {}
  auto delayIrq() -> void override {}

  void step();
  void reset();

  void skipDMACycles();

  void interrupt(InterruptType type);

 private:

  int m_skipCycles;
  int m_cycles;

  bool f_N;

  bool m_pendingNMI;
  bool m_pendingIRQ;

  MainBus &m_bus;
};

};
#endif // CPU_H
