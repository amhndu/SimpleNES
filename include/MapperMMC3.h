#pragma once
#include "Mapper.h"

namespace sn
{

  class MapperMMC3 : public Mapper
  {
  public:
    MapperMMC3(Cartridge &cart, std::function<void(void)> mirroring_cb);

    Byte readPRG(Address addr);
    void writePRG(Address addr, Byte value);
    const Byte *getPagePtr(Address addr);
    NameTableMirroring getNameTableMirroring();
    Byte readCHR(Address addr);
    void writeCHR(Address addr, Byte value);
    bool irqState();
    void irqClear();
    void scanline();

  private:
    // Control variables
    uint32_t nTargetRegister;
    bool bPRGBankMode;
    bool bCHRInversion;
    NameTableMirroring mirrormode;

    uint32_t pRegister[8];
    Byte chram[0x2000];
    Byte ppuram[0x800];
    Byte banks;
    Byte lastread;
    bool bIRQActive;
    bool bIRQEnable;
    Address nIRQCounter;
    Address nIRQReload;
    bool m_irqReloadPending;

    const Byte *prgbank0;
    const Byte *prgbank1;
    const Byte *prgbank2;
    const Byte *prgbank3;

    const Byte *chrbank0;
    const Byte *chrbank1;
    const Byte *chrbank2;
    const Byte *chrbank3;
    const Byte *chrbank4;
    const Byte *chrbank5;
    const Byte *chrbank6;
    const Byte *chrbank7;

    std::vector<Byte> ramstatic;
    std::function<void(void)> m_mirroringCallback;
  };

} // namespace sn
