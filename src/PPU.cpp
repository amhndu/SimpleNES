#include "PPU.h"

namespace sn
{
    PPU::PPU(PictureBus& bus, VirtualScreen& screen) :
        m_bus(bus),
        m_screen(screen),
    {}

    void PPU::reset()
    {
        m_longSprites = m_vblankInterrupt = m_greyscaleMode = m_vblank = false;
        m_showBackground = m_showSprites = true;
        m_bgPage = m_sprPage = Low;
        m_dataAddrIncrement = m_baseNameTable = m_dataAddress = 0;
    }

    void PPU::step()
    {

    }

    void PPU::control(Byte ctrl)
    {
        m_vblankInterrupt = ctrl & 0x80;
        m_longSprites = ctrl & 0x20;
        m_bgPage = static_cast<CharacterPage>(!!(ctrl & 0x10));
        m_sprPage = static_cast<CharacterPage>(!!(ctrl & 0x8));
        m_dataAddrIncrement = 0x8 * (ctrl & 0x4); //result will be 0x20 if bit is set, 0 otherwise
        m_baseNameTable = ctrl & 0x3;
    }

    void PPU::setMask(Byte mask)
    {
        m_greyscaleMode = mask & 0x1;
        m_showBackground = mask & 0x8;
        m_showSprites = mask & 0x10;
        //Other other unimplemented bits
    }

    Byte PPU::getStatus()
    {
        Byte status = m_sprZeroHit << 6 |
                      m_vblank << 7;
        m_dataAddress = 0;
        m_vblank = false;
        //scroll = 0
        return status;
    }

    void PPU::setDataAddress(Byte addr)
    {
        m_dataAddress = ((m_dataAddress << 8) & 0xff00) | addr;
    }

    Byte PPU::getData()
    {
        return m_bus.read(m_dataAddress);
    }

    Byte PPU::getOAMData()
    {

    }

    void PPU::setData(Byte data)
    {
        m_bus.write(m_dataAddress, data);
    }

    void PPU::setOAMAddress(Byte addr)
    {

    }

    void PPU::setScroll(Byte scroll)
    {
        //reset in getStatus
    }

}
