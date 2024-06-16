#pragma once
#include "Mapper.h"
#include <array>

namespace sn
{

  class MapperMMC3 : public Mapper
  {
  public:
    MapperMMC3(Cartridge &cart, std::function<void()> interrupt_cb, std::function<void(void)> mirroring_cb);

    Byte readPRG(Address addr);
    void writePRG(Address addr, Byte value);

    NameTableMirroring getNameTableMirroring();
    Byte readCHR(Address addr);
    void writeCHR(Address addr, Byte value);

    void scanlineIRQ();

  private:
    // Control variables
    uint32_t m_targetRegister;
    bool m_prgBankMode;
    bool m_chrInversion;

    uint32_t m_bankRegister[8];

    bool m_irqEnabled;
    Byte m_irqCounter;
    Byte m_irqLatch;
    bool m_irqReloadPending;

    std::vector<Byte> m_prgRam;
    std::vector<Byte> m_mirroringRam;
    const Byte *m_prgBank0;
    const Byte *m_prgBank1;
    const Byte *m_prgBank2;
    const Byte *m_prgBank3;

    std::array<uint32_t, 8> m_chrBanks;

    NameTableMirroring m_mirroring;
    std::function<void(void)> m_mirroringCallback;
    std::function<void()> m_interruptCallback;
  };

} // namespace sn
