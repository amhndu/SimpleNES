#include "APU/Units.h"
#include "Log.h"

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
    return !enabled || counter == 0;
}

/***** LinearCounter ****/
void LinearCounter::set_linear(int new_value)
{
    reloadValue = new_value;
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

/***** Volume *****/

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

}
