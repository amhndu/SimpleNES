#include "PPU.h"
#include "Log.h"

namespace sn
{
    PPU::PPU(PictureBus& bus, VirtualScreen& screen) :
        m_bus(bus),
        m_screen(screen)
    {}

    void PPU::reset()
    {
        m_longSprites = m_generateInterrupt = m_greyscaleMode = m_vblank = false;
        m_showBackground = m_showSprites = m_evenFrame = true;
        m_bgPage = m_sprPage = Low;
        m_dataAddress = m_cycle = m_scanline = 0;
        m_baseNameTable = 0x2000;
        m_dataAddrIncrement = 1;
        m_pipelineState = PreRender;
    }

    void PPU::setInterruptCallback(std::function<void(void)> cb)
    {
        m_vblankCallback = cb;
    }

    void PPU::step()
    {
        switch (m_pipelineState)
        {
            case PreRender:
                if ((m_cycle >= ScanlineEndCycle && m_evenFrame) || //if rendering is on, every other frame is one cycle shorter
                    (m_cycle == ScanlineEndCycle - 1 && !m_evenFrame && m_showBackground && m_showSprites))
                {
                    m_pipelineState = Render;
                    m_cycle = m_scanline = 0;
                }
                break;

            case Render:
                if (m_showBackground && m_cycle > 0 && m_cycle <= ScanlineVisibleDots)
                {
                    int x = m_cycle - 1;
                    int y = m_scanline;

                    //fetch tile number from nametable
                    Address addr = m_baseNameTable + x / 8 + (y / 8) * (ScanlineVisibleDots / 8);
                    Byte tile = read(addr);

                    //fetch pattern and calculate lower two bits
                    addr = tile * 16 + y % 8;
                    if (m_bgPage == High) addr += 0x1000;
                    Byte color = (read(addr) >> (7 - x % 8)) & 1; //bit 0 of palette entry
                    color |= ((read(addr + 8) >> (7 - x % 8)) & 1) << 1; //bit 1

                    //fetch attribute and calculate higher two bits of palette
                    addr = m_baseNameTable + AttributeOffset + x / 32 + (y * 8) / 32;
                    Byte attribute = read(addr);
                    int shift = ((((y % 32) / 16) << 1) + (x % 32) / 16) << 1;
                    color |= (attribute >> shift) << 2;

                    m_screen.setPixel(x, y, sf::Color(colors[read(0x3f00 | color)]));
                }

                if (m_cycle >= ScanlineEndCycle)
                {
                    ++m_scanline;
                    m_cycle = 0;
                }

                if (m_scanline >= VisibleScanlines)
                    m_pipelineState = PostRender;

                break;
            case PostRender:
                if (m_cycle >= ScanlineEndCycle)
                {
                    ++m_scanline;
                    m_cycle = 0;
                    m_pipelineState = VerticalBlank;
                }

                break;
            case VerticalBlank:
                if (m_cycle == 1)
                {
                    m_vblank = true;
                    if (m_generateInterrupt) m_vblankCallback();
                }
                else if (m_cycle >= ScanlineEndCycle)
                {
                    ++m_scanline;
                    m_cycle = 0;
                }

                if (m_scanline >= FrameEndScanline)
                {
                    m_pipelineState = PreRender;
                    m_scanline = 0;
                    m_vblank = false;
                }

                break;
            default:
                LOG(Error) << "Well, this shouldn't have happened." << std::endl;
        }

        ++m_cycle;
    }

    void PPU::control(Byte ctrl)
    {
        m_generateInterrupt = ctrl & 0x80;
        m_longSprites = ctrl & 0x20;
        m_bgPage = static_cast<CharacterPage>(!!(ctrl & 0x10));
        m_sprPage = static_cast<CharacterPage>(!!(ctrl & 0x8));
        if (ctrl & 0x4)
            m_dataAddrIncrement = 0x20;
        else
            m_dataAddrIncrement = 1;
        //m_dataAddrIncrement = 1 << (5 * (((ctrl & 0x4) >> 2) & 1)); //result will be 0x20 if bit is set, 1 otherwise
        m_baseNameTable = (ctrl & 0x3) * 0x400 + 0x2000;
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
        auto data = m_bus.read(m_dataAddress);
        m_dataAddress += m_dataAddrIncrement;
        return data;
    }

    Byte PPU::getOAMData()
    {
        return 0;
    }

    void PPU::setData(Byte data)
    {
        m_bus.write(m_dataAddress, data);
        m_dataAddress += m_dataAddrIncrement;
    }

    void PPU::setOAMAddress(Byte addr)
    {

    }

    void PPU::setScroll(Byte scroll)
    {
        //reset in getStatus
    }

    Byte PPU::read(Address addr)
    {
        return m_bus.read(addr);
    }

}
