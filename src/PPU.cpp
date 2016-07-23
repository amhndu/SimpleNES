#include "PPU.h"
#include "Log.h"

namespace sn
{
    PPU::PPU(PictureBus& bus, VirtualScreen& screen) :
        m_bus(bus),
        m_screen(screen),
        m_spriteMemory(64 * 4)
    {}

    void PPU::reset()
    {
        m_longSprites = m_generateInterrupt = m_greyscaleMode = m_vblank = false;
        m_showBackground = m_showSprites = m_evenFrame = true;
        m_bgPage = m_sprPage = Low;
        m_dataAddress = m_cycle = m_scanline = m_spriteDataAddress = 0;
        m_baseNameTable = 0x2000;
        m_dataAddrIncrement = 1;
        m_pipelineState = PreRender;
        m_scanlineSprites.reserve(8);
        m_scanlineSprites.resize(0);
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
                if (m_cycle == 1)
                    m_vblank = false;

                if ((m_cycle >= ScanlineEndCycle && m_evenFrame) || //if rendering is on, every other frame is one cycle shorter
                    (m_cycle == ScanlineEndCycle - 1 && !m_evenFrame && m_showBackground && m_showSprites))
                {
                    m_pipelineState = Render;
                    m_cycle = m_scanline = 0;
                }
                break;
                //TODO Caching of tile/attribute/nametable data
            case Render:
                if (m_cycle > 0 && m_cycle <= ScanlineVisibleDots)
                {
                    Byte bgColor = 0, sprColor = 0;

                    bool spritePriorityHigh = false;

                    int x = m_cycle - 1;
                    int y = m_scanline;

                    if (m_showBackground)
                    {
                        //fetch tile number from nametable
                        Address addr = m_baseNameTable + x / 8 + (y / 8) * (ScanlineVisibleDots / 8);
                        Byte tile = read(addr);

                        //fetch pattern and calculate lower two bits
                        addr = tile * 16 + y % 8;
                        if (m_bgPage == High) addr += 0x1000;
                        bgColor = (read(addr) >> (7 ^ x % 8)) & 1; //bit 0 of palette entry
                        bgColor |= ((read(addr + 8) >> (7 ^ x % 8)) & 1) << 1; //bit 1

                        //fetch attribute and calculate higher two bits of palette
                        addr = m_baseNameTable + AttributeOffset + x / 32 + ((y / 32) * 8);
                        Byte attribute = read(addr);
                        int shift = ((((y % 32) / 16) << 1) + (x % 32) / 16) << 1;

                        bgColor |= ((attribute >> shift) & 0x3) << 2;
                    }

                    if (m_showSprites)
                    {
                        for (auto i : m_scanlineSprites)
                        {
                            Byte spr_x =     m_spriteMemory[i * 4 + 3];

                            if (0 > x - spr_x || x - spr_x >= 8)
                                continue;

                            Byte spr_y     = m_spriteMemory[i * 4 + 0] + 1,
                                 tile      = m_spriteMemory[i * 4 + 1],
                                 attribute = m_spriteMemory[i * 4 + 2];

                            int length = (m_longSprites) ? 16 : 8;

                            int x_shift = (x - spr_x) % 8, y_offset = (y - spr_y) % length;

                            if ((attribute & 0x40) == 0) //If NOT flipping horizontally
                                x_shift ^= 7;
                            if ((attribute & 0x80) != 0) //IF flipping vertically
                                y_offset ^= (length - 1);

                            Address addr = tile * 16 + y_offset;
                            if (m_sprPage == High) addr += 0x1000;

                            sprColor = 0x10; //Select sprite palette
                            sprColor |= (read(addr) >> (x_shift)) & 1; //bit 0 of palette entry
                            sprColor |= ((read(addr + 8) >> (x_shift)) & 1) << 1; //bit 1
                            sprColor |= (attribute & 0x3) << 2; //bits 2-3

                            spritePriorityHigh = !(attribute & 0x20);

                            break; //Exit now since we've found the highest priority sprite
                        }
                    }

                    Byte paletteAddr = bgColor;

                    if ( ((bgColor & 0x3) == 0 && (sprColor & 0x3) != 0) ||
                         ((bgColor & 0x3) != 0 && (sprColor & 0x3) != 0) )
                        paletteAddr = sprColor;
                    //else bgColor

//                     if (spritesFound)
//                         paletteAddr = sprColor;

                    m_screen.setPixel(x, y, sf::Color(colors[m_bus.readPalette(paletteAddr)]));
                }

                if (m_cycle >= ScanlineEndCycle)
                {
                    //Find and index sprites that are on the next Scanline
                    //This isn't where/when this indexing, actually copying in 2C02 is done
                    //but (I think) it shouldn't hurt any games if this is done here

                    m_scanlineSprites.resize(0);

                    int range = 8;
                    if (m_longSprites)
                        range = 16;

                    std::size_t j = 0;
                    for (std::size_t i = 0; i < 64; ++i)
                    {
                        auto diff = (m_scanline - m_spriteMemory[i * 4]);
                        if (0 <= diff && diff < range)
                        {
                            m_scanlineSprites.push_back(i);
                            ++j;
                            if (j >= 8)
                            {
                                //TODO overflow bit handling
                                break;
                            }
                        }
                    }

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
//                     m_vblank = false;
                }

                break;
            default:
                LOG(Error) << "Well, this shouldn't have happened." << std::endl;
        }

        ++m_cycle;
    }

    Byte PPU::readOAM(Byte addr)
    {
        return m_spriteMemory[addr];
    }

    void PPU::writeOAM(Byte addr, Byte value)
    {
        m_spriteMemory[addr] = value;
    }

    void PPU::doDMA(const Byte* page_ptr)
    {
        LOG(Info) << "DMA Address: " << +m_spriteDataAddress << std::endl;
        std::memcpy(m_spriteMemory.data(), page_ptr, 256);
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

        //Reads are delayed by one byte/read when address is in this range
        if (m_dataAddress < 0x3f00)
        {
            //Return from the data buffer and store the current value in the buffer
            std::swap(data, m_dataBuffer);
        }

        return data;
    }

    Byte PPU::getOAMData()
    {
        return readOAM(m_spriteDataAddress++);
    }

    void PPU::setData(Byte data)
    {
        m_bus.write(m_dataAddress, data);
        m_dataAddress += m_dataAddrIncrement;
    }

    void PPU::setOAMAddress(Byte addr)
    {
        m_spriteDataAddress = addr;
    }

    void PPU::setOAMData(Byte value)
    {
        writeOAM(m_spriteDataAddress++, value);
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
