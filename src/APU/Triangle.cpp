#include "APU/Triangle.h"
#include "APU/Divider.h"
#include "APU/Units.h"
#include "Cartridge.h"
#include "Log.h"
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>

namespace sn
{

inline int calc_note_freq(int period, int seq_length, nanoseconds clock_period)
{
    return 1e9 / ((float)clock_period.count() * period * seq_length);
}

void Triangle::set_period(int p)
{
    period = p;
    sequencer.set_period(p);
}

// Clocked at the cpu freq
void Triangle::clock()
{
    if (length_counter.muted())
    {
        return;
    }

    if (linear_counter.counter == 0)
    {
        return;
    }

    if (sequencer.clock())
    {
        seq_idx = (seq_idx + 1) % 32;
    }
}

Byte Triangle::sample() const
{
    return volume();
}

int Triangle::volume() const
{
    // const static int sequence[] {
    //     // clang-format off
    //     15, 14, 13, 12, 11, 10, 9, 8, 7, 6,  5,  4,  3,  2,  1,  0,
    //     0,   1,  2,  3,  4,  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    //     // clang-format on
    // };
    // return sequence[seq_idx];

    if (seq_idx < 16)
    {
        return 15 - seq_idx;
    }
    else
    {
        return seq_idx - 16;
    }
}

}
