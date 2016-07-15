#ifndef PPU_H
#define PPU_H
#include "PictureBus.h"
#include "MainBus.h"

namespace sn
{
    class PPU
    {
        public:
            PPU(PictureBus &bus);
            void step();

            //Callbacks mapped to CPU address space
            //Write
            void control(Byte ctrl);
            void setMask(Byte mask);
            void setOAMAddress(Byte Address);
            void setScroll(Byte scroll);
            void setData(Byte data);
            //Read
            Byte getStatus();
            Byte getData();
            Byte getOAMData();
        private:
            PictureBus &m_bus;
    };
}

#endif // PPU_H
