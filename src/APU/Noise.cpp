#include "APU/Noise.h"
#include "APU/Divider.h"
#include "Cartridge.h"
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>

namespace sn
{

void Noise::set_period_from_table(int idx)
{
    const static int periods[] {
        4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
    };

    divider.set_period(periods[idx]);
}

void Noise::clock()
{
    if (!divider.clock())
    {
        return;
    }

    bool feedback_input1 = (shift_register & 0x2) ? mode == Bit1 : (shift_register & 0x40);
    bool feedback_input2 = (shift_register & 0x1);

    bool feedback        = feedback_input1 != feedback_input2;

    shift_register       = shift_register >> 1 | (feedback << 14);
}

Byte Noise::sample() const
{
    return volume.get();
}

}
