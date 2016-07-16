#include "PPU.h"

namespace sn
{
    PPU::PPU(PictureBus& bus, VirtualScreen& screen) :
        m_bus(bus),
        m_screen(screen),
        m_longSprites(0),
        m_vblankInterrupt(false),
        m_greyscaleMode(false),
        m_showSprites(true),
        m_showBackground(true),
        m_bgPage(Low),
        m_sprPage(High),
        m_dataAddrIncrement(0),
        m_baseNameTable(0)
    {}

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
        //TODO later
        return 0;
    }

    Byte PPU::getData()
    {

    }

    Byte PPU::getOAMData()
    {

    }

    void PPU::setData(Byte data)
    {

    }

    void PPU::setOAMAddress(Byte Address)
    {

    }

    void PPU::setScroll(Byte scroll)
    {

    }
    
    void PPU::step()
    {

    }


}
