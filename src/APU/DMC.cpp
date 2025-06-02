#include "APU/DMC.h"
#include "APU/Divider.h"
#include "Cartridge.h"
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>

namespace sn
{

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

        current_address = sample_begin;
        remaining_bytes = sample_length;
    }
    else
    {
        remaining_bytes -= 1;
    }

    sample_buffer = dma(current_address);

    if (current_address == 0xffff)
    {
        current_address = 0x8000;
    }
    else
    {
        current_address += 1;
    }
    return true;
}

}
