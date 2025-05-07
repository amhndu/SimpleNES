#include "APU/spsc.hpp"
#include "Cartridge.h"
#include "Log.h"
#include "miniaudio.h"

#include "APU/Divider.h"
#include "APU/Pulse.h"
#include "APU/Timer.h"
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>
#include <functional>

namespace sn
{
/***** LengthCounter *****/

void LengthCounter::set_enable(bool new_value)
{
    enabled = new_value;

    if (!enabled)
    {
        counter = 0;
    }
}

void LengthCounter::set_linear(int new_value)
{
    if (!enabled)
    {
        return;
    }

    counter = new_value;
}

void LengthCounter::set_from_table(std::size_t index)
{
    const int length_table[] = {
        10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
        12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
    };

    set_linear(length_table[index]);
}

void LengthCounter::half_frame_clock()
{
    if (halt)
    {
        return;
    }

    if (counter == 0)
    {
        return;
    }

    --counter;
}

bool LengthCounter::muted() const
{
    return counter == 0;
}

/***** EnvelopeGenerator *****/

void Volume::quarter_frame_clock()
{
    if (shouldStart)
    {
        shouldStart = false;
        decayVolume = max_volume;
        divider.reset(fixedVolumeOrPeriod);
        return;
    }

    if (!divider.clock())
    {
        return;
    }

    if (decayVolume > 0)
    {
        --decayVolume;
    }
    else if (isLooping)
    {
        decayVolume = max_volume;
    }
}

int Volume::get() const
{
    if (constantVolume)
    {
        return fixedVolumeOrPeriod;
    }

    return decayVolume;
}

/***** Pulse::Sweep *****/

void Pulse::Sweep::half_frame_clock()
{
    if (reload)
    {
        divider.reset(period);
        reload = false;
        return;
    }

    if (!divider.clock())
    {
        return;
    }

    if (enabled && shift > 0)
    {
        if (!pulse.sweep_muted)
        {
            const auto current = pulse.sequencer.get_period();
            pulse.period       = calculate_target(current);
            pulse.reload_period();
        }
    }
};

int Pulse::Sweep::calculate_target(int current) const
{
    const auto amt = current >> shift;
    if (!negate)
    {
        return current + amt;
    }

    if (ones_complement)
    {
        return std::max(0, current - amt);
    }

    return std::max(0, current - amt - 1);
}

/***** Pulse *****/

void Pulse::reload_period()
{
    sequencer.reset(period);
    auto target = sweep.calculate_target(period);
    sweep_muted = sweep.is_muted(period, target);
}

void Pulse::set_frequency(double output_freq)
{
    // output_f = divider_freq / 8 = clock_freq / (8 * divider_p) = 1 / (8 * divider_p * clock_period)
    // divider_p = 1 / (8 * clock_period * output_f)
    auto period = 1.0 / (Duty::Length * apu_clock_period_s.count() * output_freq);
    sequencer.reset(static_cast<int>(period));
    LOG(sn::Info) << "pulse t: " << sequencer.get_period() << std::endl;
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

    if (sweep_muted)
    {
        return 0;
    }

    if (!Duty::active(seq_type, seq_idx))
    {
        return 0;
    }

    return volume.get();
}
}
