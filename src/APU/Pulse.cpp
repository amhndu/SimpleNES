#include "APU/Constants.h"
#include "Cartridge.h"
#include "Log.h"

#include "APU/Divider.h"
#include "APU/Pulse.h"
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

void LengthCounter::set_from_table(std::size_t index)
{
    const static int length_table[] = {
        10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
        12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
    };

    if (!enabled)
    {
        return;
    }

    counter = length_table[index];
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
        divider.set_period(fixedVolumeOrPeriod);
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
        divider.set_period(period);
        reload = false;
        return;
    }

    if (!divider.clock())
    {
        return;
    }

    if (enabled && shift > 0)
    {
        int current = pulse.period;
        int target  = calculate_target(current);
        if (!is_muted(pulse.period, target))
        {
            pulse.set_period(target);
        }
    }
}

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

int calc_note_freq(int period, int seq_length, nanoseconds clock_period)
{
    return 1e9 / ((float)clock_period.count() * period * seq_length);
}

void Pulse::set_period(int p)
{
    period = p;
    sequencer.set_period(period);

    auto note_freq = calc_note_freq(period, 8, apu_clock_period_ns);
    LOG(CpuTrace) << "PULSE RELEAD" << (int)type << VAR_PRINT(period) << std::hex << VAR_PRINT(note_freq) << std::dec
                  << std::endl;
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

    if (!Duty::active(seq_type, seq_idx))
    {
        return 0;
    }

    return volume.get();
}

/***** LengthCounter ****/
void LinearCounter::set_linear(int new_value)
{
    counter = new_value;
}

void LinearCounter::quarter_frame_clock()
{
    if (reload)
    {
        counter = reloadValue;
        if (!control)
        {
            reload = false;
        }
    }

    if (counter == 0)
    {
        return;
    }

    --counter;
}

/**** Triangle ****/

void Triangle::set_period(int p)
{
    period = p;
    sequencer.set_period(p);

    auto note_freq = calc_note_freq(period, 32, cpu_clock_period_ns);
    LOG(CpuTrace) << "TRIANGLE RELEAD" << VAR_PRINT(period) << std::hex << VAR_PRINT(note_freq) << std::dec
                  << std::endl;
}

// Clocked at the cpu freq
void Triangle::clock()
{
    if (linear_counter.counter == 0 || length_counter.counter == 0)
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
    if (length_counter.muted())
    {
        return 0;
    }

    if (linear_counter.counter == 0)
    {
        return 0;
    }

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
    if (length_counter.muted())
    {
        return 0;
    }

    if (shift_register & 0x1)
    {
        return 0;
    }

    return volume.get();
}

/**** DMC ****/
void DMC::set_irq_enable(bool enable)
{
    irqEnable = enable;
    if (!irqEnable)
    {
        interrupt = false;
        irq.release();
    }
}

void DMC::control(bool enable)
{
    change_enabled = enable;
    if (!enable)
    {
        remaining_bytes = 0;
    }
    else if (remaining_bytes == 0)
    {
        // restart
        current_address = sample_begin;
        remaining_bytes = sample_length;
    }
}

void DMC::set_rate(int idx)
{
    const static int rate[] { 428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54 };

    change_rate.set_period(rate[idx]);
    change_rate.reset();
}

void DMC::clear_interrupt()
{
    irq.release();
    interrupt = false;
}

Byte DMC::sample() const
{
    return volume;
}

void DMC::clock()
{
    if (!change_enabled)
    {
        return;
    }

    if (!change_rate.clock())
    {
        return;
    }

    int delta = pop_delta();
    if (silenced)
    {
        return;
    }

    if (delta == 1 && volume <= 125)
    {
        volume += 2;
    }
    else if (delta == 0 && volume >= 2)
    {
        volume -= 2;
    }

    // LOG(InfoVerbose) << "APU_DMC_VOLUME_CHANGE" << std::dec << VAR_PRINT(volume) << VAR_PRINT(delta) << std::dec
    //                  << VAR_PRINT(remaining_bytes) << std::endl;
}

int DMC::pop_delta()
{
    if (remaining_bits == 0)
    {
        remaining_bits = 8;

        if (load_sample())
        {
            shifter  = sample_buffer;
            silenced = false;
        }
        else
        {
            silenced = true;
        }
    }
    else
    {
        --remaining_bits;
    }

    int rv    = shifter & 0x1;
    shifter >>= 1;
    return rv;
}

bool DMC::load_sample()
{
    if (remaining_bytes == 0)
    {
        if (!loop)
        {
            if (irqEnable)
            {
                interrupt = true;
                irq.pull();
            }

            return false;
        }

        // LOG(InfoVerbose) << "APU_DMC_LOOP_RESTART" << std::endl;
        current_address = sample_begin;
        remaining_bytes = sample_length;
    }
    else
    {
        remaining_bytes -= 1;
    }

    // LOG(InfoVerbose) << "APU_DMC_DMA" << std::hex << VAR_PRINT(current_address) << VAR_PRINT(sample_begin)
    //                  << VAR_PRINT(sample_begin + sample_length) << std::dec << std::endl;
    sample_buffer    = dma(current_address);

    current_address += 1;
    if (current_address > 0xffff)
    {
        current_address = 0x8000 + (current_address - (0xffff + 1));
    }
    return true;
}

}
