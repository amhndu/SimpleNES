#include "PPU.h"
#include "Log.h"

namespace sn
{
    PPU::PPU(PictureBus& bus, VirtualScreen& screen) :
        m_bus(bus),
        m_screen(screen),
    {}

    void PPU::reset()
    {
        m_longSprites = m_vblankInterrupt = m_greyscaleMode = m_vblank = false;
        m_showBackground = m_showSprites = m_evenFrame = true;
        m_bgPage = m_sprPage = Low;
        m_dataAddress = m_cycle = m_scanline = 0;
        m_baseNameTable = 0x2000;
        m_dataAddrIncrement = 1;
        m_pipelineState = PreRender;
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
                    //fetch tile number from nametable
                    Address addr = m_baseNameTable + m_cycle / 8 + m_scanline * VisibleScanlines / 8;
                    Byte tile = read(addr);

                    //fetch pattern and calculate lower two bits
                    addr = tile * 16 + m_scanline % 8;
                    if (m_bgPage == High) addr += 0x1000;
                    Byte color = (read(addr) >> (m_cycle % 8)) & 1;
                    color |= ((read(addr + 8) >> (m_cycle % 8)) & 1) << 1;

                    //fetch attribute and calculate higher two bits of palette
                    addr = m_baseNameTable + AttributeOffset + m_cycle / 32 + m_scanline * 8 / 32;
                    Byte attribute = read(addr);
                    int shift = (((m_scanline % 32) / 16) << 1 + (m_cycle % 32) / 16) << 1;
                    color |= (attribute >> shift) << 2;

                    m_screen.setPixel(m_cycle, m_scanline, sf::Color(colors[read(color)]));
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
                if (m_cycle == 0 && m_vblankInterrupt)
                    ;//generate NMI
                if (m_cycle >= ScanlineEndCycle)
                {
                    ++m_scanline;
                    m_cycle = 0;
                }

                if (m_scanline >= FrameEndScanline)
                {
                    m_pipelineState = PreRender;
                    m_scanline = 0;
                }

                break;
            default:
                LOG(Error) << "Well, this shouldn't have happened." << std::endl;
        }

        ++m_cycle;
    }

    void PPU::control(Byte ctrl)
    {
        m_vblankInterrupt = ctrl & 0x80;
        m_longSprites = ctrl & 0x20;
        m_bgPage = static_cast<CharacterPage>(!!(ctrl & 0x10));
        m_sprPage = static_cast<CharacterPage>(!!(ctrl & 0x8));
        m_dataAddrIncrement = 0x8 * (ctrl & 0x4); //result will be 0x20 if bit is set, 0 otherwise
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

    Byte PPU::read(Address addr)
    {
        return m_bus.read(addr);
    }

}
