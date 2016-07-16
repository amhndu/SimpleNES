#ifndef PPU_H
#define PPU_H
#include "PictureBus.h"
#include "MainBus.h"
#include "VirtualScreen.h"

namespace sn
{
    class PPU
    {
        public:
            PPU(PictureBus &bus, VirtualScreen &screen);
            void step();

            //Callbacks mapped to CPU address space
            //Addresses written to by the program
            void control(Byte ctrl);
            void setMask(Byte mask);
            void setOAMAddress(Byte Address);
            void setScroll(Byte scroll);
            void setData(Byte data);
            //Read by the program
            Byte getStatus();
            Byte getData();
            Byte getOAMData();
        private:
            PictureBus &m_bus;
            VirtualScreen &m_screen;

            //Flags and setup variables
            bool m_longSprites;
            bool m_vblankInterrupt;
            bool m_greyscaleMode;
            bool m_showSprites;
            bool m_showBackground;
            enum CharacterPage
            {
                Low,
                High,
            } m_bgPage, m_sprPage;
            Address m_dataAddrIncrement;
            Byte m_baseNameTable;
    };
}

#endif // PPU_H
