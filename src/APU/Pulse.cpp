#include "APU/Constants.h"
#include "Cartridge.h"
#include "Log.h"

#include "APU/Divider.h"
#include "APU/Pulse.h"
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>

namespace sn
{

/***** Pulse *****/

inline int calc_note_freq(int period, int seq_length, nanoseconds clock_period)
{
    return 1e9 / ((float)clock_period.count() * period * seq_length);
}

void Pulse::set_period(int p)
{
    period = p;
    sequencer.set_period(period);
}

// Clocked at half the cpu freq
void Pulse::clock()
{
    if (sequencer.clock())
    {
        // NES counts downwards in sequencer
        seq_idx = (8 + (seq_idx - 1)) % 8;
    }
}

Byte Pulse::sample() const
{
    if (length_counter.muted())
    {
        return 0;
    }

    // TODO: cache the target to avoid recalculation?
    if (sweep.is_muted(period, sweep.calculate_target(period)))
    {
        LOG(InfoVerbose) << "sweep muted" << VAR_PRINT(period) << VAR_PRINT(sweep.calculate_target(period))
                         << std::endl;
        return 0;
    }

    if (!PulseDuty::active(seq_type, seq_idx))
    {
        return 0;
    }

    return volume.get();
}

/***** Sweep *****/

void Sweep::half_frame_clock()
{
    if (reload)
    {
        divider.set_period(period);
        reload = false;
        return;
    }

    if (!enabled)
    {
        return;
    }

    if (!divider.clock())
    {
        return;
    }

    if (true /*shift > 0*/)
    {
        int current = pulse.period;
        int target  = calculate_target(current);
        if (!is_muted(pulse.period, target))
        {
            LOG(ApuTrace) << "Sweep update" << " " << (int)pulse.type << VAR_PRINT(+current) << VAR_PRINT(+target)
                          << std::endl;
            pulse.set_period(target);
        }
        else
        {
            LOG(ApuTrace) << "Sweep mute skip" << " " << (int)pulse.type << VAR_PRINT(+current) << VAR_PRINT(+target)
                          << std::endl;
        }
    }
}

int Sweep::calculate_target(int current) const
{
    const auto amt = current >> shift;
    if (!negate)
    {
        return current + amt;
    }

    if (ones_complement)
    {
        return std::max(0, current - amt - 1);
    }

    return std::max(0, current - amt);
}

}
