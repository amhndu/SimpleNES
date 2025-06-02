#pragma once

#include <cstddef>
#include <cstdint>

#include "APU/Constants.h"
#include "APU/Divider.h"
#include "APU/FrameCounter.h"

namespace sn
{

struct LengthCounter : public FrameClockable
{
    void set_enable(bool new_value);
    bool is_enabled() const { return enabled; }

    void set_from_table(std::size_t index);
    void half_frame_clock() override;
    bool muted() const;

    bool halt    = false;

    bool enabled = false;
    int  counter = 0;
};

struct LinearCounter : public FrameClockable
{
    void set_linear(int new_value);
    void quarter_frame_clock() override;

    bool reload      = false;
    int  reloadValue = 0;
    bool control     = true;

    int  counter     = 0;
};

struct Volume : public FrameClockable
{
    void          quarter_frame_clock() override;

    int           get() const;

    Divider       divider { 0 };
    std::uint32_t fixedVolumeOrPeriod = max_volume;
    std::uint32_t decayVolume         = max_volume;
    bool          constantVolume      = true;
    bool          isLooping           = false;
    bool          shouldStart         = false;
};

}
