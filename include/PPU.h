#ifndef PPU_H
#define PPU_H
#include "PictureBus.h"
#include "MainBus.h"
#include "VirtualScreen.h"
#include "PaletteColors.h"

namespace sn
{
    const int ScanlineCycleLength = 341;
    const int ScanlineEndCycle = 340;
    const int VisibleScanlines = 240;
    const int ScanlineVisibleDots = 256;
    const int FrameEndScanline = 261;

    const int AttributeOffset = 0x3C0;

    class PPU
    {
        public:
            PPU(PictureBus &bus, VirtualScreen &screen);
            void step();
            void reset();

            //Callbacks mapped to CPU address space
            //Addresses written to by the program
            void control(Byte ctrl);
            void setMask(Byte mask);
            void setOAMAddress(Byte addr);
            void setDataAddress(Byte addr);
            void setScroll(Byte scroll);
            void setData(Byte data);
            //Read by the program
            Byte getStatus();
            Byte getData();
            Byte getOAMData();
        private:
            Byte read(Address addr);
            PictureBus &m_bus;
            VirtualScreen &m_screen;

            enum State
            {
                PreRender,
                Render,
                PostRender,
                VerticalBlank
            } m_pipelineState;
            int m_cycle;
            int m_scanline;
            bool m_evenFrame;

            bool m_vblank;
            bool m_sprZeroHit;

            Address m_dataAddress;

            //Setup flags and variables
            bool m_longSprites;
            bool m_vblankInterrupt;

            bool m_greyscaleMode;
            bool m_showSprites;
            bool m_showBackground;

            enum CharacterPage
            {
                Low,
                High,
            } m_bgPage,
              m_sprPage;

            Address m_dataAddrIncrement;
            Address m_baseNameTable;
    };
}

#endif // PPU_H
