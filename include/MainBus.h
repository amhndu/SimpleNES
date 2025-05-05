#ifndef MEMORY_H
#define MEMORY_H
#include <vector>
#include <functional>
#include "Cartridge.h"
#include "Controller.h"
#include "Mapper.h"
#include "PPU.h"

namespace sn
{
    class MainBus
    {
        public:
            enum Register
            {
                PPU_CTRL = 0x2000,
                PPU_MASK,// = 0x2001,
                PPU_STATUS,// = 0x2002,
                OAM_ADDR,// = 0x2003,
                OAM_DATA,// = 0x2004,
                PPU_SCROL,// = 0x2005,
                PPU_ADDR,// = 0x2006,
                PPU_DATA,// = 0x2007,

                // 0x2008 - 0x3fff mirrors of 0x2000 - 0x2007

                APU_SQ1_VOL = 0x4000,
                APU_SQ1_SWEEP = 0x4001,
                APU_SQ1_LO = 0x4002,
                APU_SQ1_HI = 0x4003,

                APU_SQ2_VOL = 0x4004,
                APU_SQ2_SWEEP = 0x4005,
                APU_SQ2_LO = 0x4006,
                APU_SQ2_HI = 0x4007,

                APU_TRI_LINEAR = 0x4008,
                // unused - 0x4009
                APU_TRI_LO = 0x400a,
                APU_TRI_HI = 0x400b,

                APU_NOISE_VOL = 0x400c,
                // unused - 0x400d
                //
                APU_NOISE_LO = 0x400e,
                APU_NOISE_HI = 0x400f,

                APU_DMC_FREQ = 0x4010,
                APU_DMC_RAW = 0x4011,
                APU_DMC_START = 0x4012,
                APU_DMC_LEN = 0x4013,

                OAM_DMA = 0x4014,

                APU_CONTROL_AND_STATUS = 0x4015,

                JOY1 = 0x4016,
                JOY2_AND_FRAME_CONTROL = 0x4017,
            };


            MainBus(PPU& ppu, Controller& ctrl1, Controller& ctrl2, std::function<void(Byte)> dma);
            Byte read(Address addr);
            void write(Address addr, Byte value);
            bool setMapper(Mapper* mapper);
            const Byte* getPagePtr(Byte page);
        private:
            std::vector<Byte> m_RAM;
            std::vector<Byte> m_extRAM;
            std::function<void(Byte)> m_dmaCallback;
            Mapper* m_mapper;
            PPU& m_ppu;
            Controller& m_controller1;
            Controller& m_controller2;
    };
};

#endif // MEMORY_H
