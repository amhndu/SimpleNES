#include "CPU.h"
#include "CPUOpcodes.h"
#include <iomanip>

namespace sn {
CPU::CPU(MainBus &mem) :
    m_pendingNMI(false),
    m_pendingIRQ(false),
    m_bus(mem) {}

void CPU::reset() {
  MOS6502::power();

  register_.pc.w = m_bus.read(ResetVector) | m_bus.read(ResetVector + 1) << 8;
}

void CPU::interrupt(InterruptType type) {
  switch (type) {
    case InterruptType::NMI:m_pendingNMI = true;
      break;

    case InterruptType::IRQ:m_pendingIRQ = true;
      break;

    default:break;
  }
}

void CPU::skipDMACycles() {
  m_skipCycles += 513; //256 read + 256 write + 1 dummy read
  m_skipCycles += (m_cycles & 1); //+1 if on odd cycle
}

void CPU::step() {
  ++m_cycles;
  if (m_skipCycles > 0) {
    --m_skipCycles;
    return;
  }

  runInstruction();

  // NMI has higher priority, check for it first
  if (m_pendingNMI) {
    MOS6502::interrupt(NMIVector);
    m_pendingNMI = m_pendingIRQ = false;
  } else if (m_pendingIRQ) {
    MOS6502::interrupt(IRQVector);
    m_pendingNMI = m_pendingIRQ = false;
  }
}

};

