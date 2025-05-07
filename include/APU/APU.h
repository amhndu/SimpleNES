#pragma once

#include "APU/FrameCounter.h"
#include "APU/Pulse.h"
#include "APU/Timer.h"
#include "APU/spsc.hpp"
#include "AudioPlayer.h"
#include "IRQ.h"
#include "Log.h"
#include "miniaudio.h"

namespace sn
{
class APU
{
public:
    Pulse        pulse1 { Pulse::Type::Pulse1 };
    Pulse        pulse2 { Pulse::Type::Pulse2 };
    Triangle     triangle;

    FrameCounter frame_counter;

public:
    APU(AudioPlayer& player, Irq& irq)
      : frame_counter(setup_frame_counter(irq))
      , audio_queue(player.audio_queue)
      , sampling_timer(nanoseconds(int64_t(1e9) / int64_t(player.std_sample_rate)))
    {
    }

    // Frame counter is clocked at CPU freq, while the channels are clocked at half the freq
    // Thus, APU is clocked at the same frequency as CPU
    void step();

    void writeRegister(Address addr, Byte value);
    Byte readStatus();

private:
    FrameCounter             setup_frame_counter(Irq& irq);
    bool                     halfDivider = false;

    spsc::RingBuffer<float>& audio_queue;
    Timer                    sampling_timer;
};

}
