#include "APU/Constants.h"
#include "Cartridge.h"
#include "Log.h"

#include "APU/Divider.h"
#include "APU/Pulse.h"
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>
#include <cmath>

namespace sn
{

/***** Pulse *****/

inline int calc_note_freq(int period, int seq_length, nanoseconds clock_period)
{
    return 1e9 / ((float)clock_period.count() * period * seq_length);
}

std::string freq_to_note(double freq)
{
    std::vector<std::string> notes              = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };

    double                   note_number_double = 12 * std::log2(freq / 440.0) + 49;
    int                      note_number        = static_cast<int>(std::round(note_number_double));

    int                      note_index         = (note_number - 1) % notes.size();
    if (note_index < 0)
    { // Handle negative modulo result in C++
        note_index += notes.size();
    }
    std::string note   = notes[note_index];

    int         octave = (note_number + 8) / notes.size();

    return note + std::to_string(octave);
}

void Pulse::set_period(int p)
{
    period = p;
    sequencer.set_period(period);

    if (sn::Log::get().getLevel() <= ApuTrace)
    {
        auto note_freq = calc_note_freq(period, 8, apu_clock_period_ns);
        auto note      = freq_to_note(note_freq);
        LOG(ApuTrace) << "PULSE" << (int)type << " RELEAD: " VAR_PRINT(period) << std::hex << VAR_PRINT(note)
                      << std::dec << std::endl;
    }
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

    if (shift > 0)
    {
        int current = pulse.period;
        int target  = calculate_target(current);
        if (!is_muted(pulse.period, target))
        {
            LOG(ApuTrace) << "Sweep" << (int)pulse.type << " update " << VAR_PRINT(+current) << VAR_PRINT(+target)
                          << VAR_PRINT(+shift) << VAR_PRINT((current >> shift)) << std::endl;
            pulse.set_period(target);
        }
        else
        {
            LOG(ApuTrace) << "Sweep" << (int)pulse.type << " skip " << VAR_PRINT(+current) << VAR_PRINT(+target)
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
