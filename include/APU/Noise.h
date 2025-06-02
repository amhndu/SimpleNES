#pragma once

#include "APU/Divider.h"
#include "APU/Units.h"
#include "Cartridge.h"

namespace sn
{

struct Noise
{
    Volume        volume;
    LengthCounter length_counter;
    Divider       divider { 0 };

    enum Mode : bool
    {
        Bit1 = 0,
        BIt6 = 1,
    } mode              = Bit1;
    int  period         = 0;
    int  shift_register = 1;

    void set_period_from_table(int idx);

    // Clocked at the cpu freq
    void clock();

    Byte sample() const;
};
}
