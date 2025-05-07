#include "APU/APU.h"
#include "APU/Constants.h"
#include "APU/Divider.h"
#include "APU/FrameCounter.h"
#include "APU/Pulse.h"
#include "APU/Timer.h"
#include "APU/spsc.hpp"
#include "AudioPlayer.h"
#include "Cartridge.h"
#include "Log.h"

#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>
#include <functional>
#include <vector>

using namespace std::chrono;

namespace sn
{
enum Register
{
    APU_SQ1_VOL       = 0x4000,
    APU_SQ1_SWEEP     = 0x4001,
    APU_SQ1_LO        = 0x4002,
    APU_SQ1_HI        = 0x4003,

    APU_SQ2_VOL       = 0x4004,
    APU_SQ2_SWEEP     = 0x4005,
    APU_SQ2_LO        = 0x4006,
    APU_SQ2_HI        = 0x4007,

    APU_TRI_LINEAR    = 0x4008,
    // unused - 0x4009
    APU_TRI_LO        = 0x400a,
    APU_TRI_HI        = 0x400b,

    APU_NOISE_VOL     = 0x400c,
    // unused - 0x400d
    APU_NOISE_LO      = 0x400e,
    APU_NOISE_HI      = 0x400f,

    APU_DMC_FREQ      = 0x4010,
    APU_DMC_RAW       = 0x4011,
    APU_DMC_START     = 0x4012,
    APU_DMC_LEN       = 0x4013,

    APU_CONTROL       = 0x4015,

    APU_FRAME_CONTROL = 0x4017,
};

void APU::step()
{
    frame_counter.clock();

    if (halfDivider)
    {
        pulse1.clock();
        pulse2.clock();
    }
    halfDivider = !halfDivider;

    for (int clocks = sampling_timer.clock(cpu_clock_period_ns); clocks > 0; --clocks)
    {
        float frame =
          (static_cast<float>(pulse1.sample()) / max_volume_f + static_cast<float>(pulse2.sample()) / max_volume_f) /
          2.0;
        audio_queue.push(frame);
    }
}

void APU::writeRegister(Address addr, Byte value)
{
    switch (addr)
    {
        break;

    case APU_SQ1_VOL:
        // DDlcvvvv
        pulse1.volume.fixedVolumeOrPeriod = value & 0xf;
        pulse1.volume.constantVolume      = value & (1 << 4);
        pulse1.length_counter.halt        = value & (1 << 5);
        pulse1.seq_type                   = static_cast<Pulse::Duty::Type>(value >> 6);
        break;
    case APU_SQ1_SWEEP:
        // EPPP.NSSS
        pulse1.sweep.enabled = value & (1 << 7);
        pulse1.sweep.period  = (value >> 4) & 0x7;
        pulse1.sweep.negate  = value & (1 << 3);
        pulse1.sweep.shift   = value & 0x7;
        pulse1.sweep.reload  = true;
        break;
    case APU_SQ1_LO:
        pulse1.period = (pulse1.period & 0xf0) | (value << 0);
        break;
    case APU_SQ1_HI:
        pulse1.period = (pulse1.period & 0x0f) | (value << 4);
        pulse1.reload_period();
        break;

    case APU_SQ2_VOL:
        pulse2.volume.fixedVolumeOrPeriod = value & 0xf;
        pulse2.volume.constantVolume      = value & (1 << 4);
        pulse2.length_counter.halt        = value & (1 << 5);
        pulse2.seq_type                   = static_cast<Pulse::Duty::Type>(value >> 6);
        break;
    case APU_SQ2_SWEEP:
        pulse2.sweep.enabled = value & (1 << 7);
        pulse2.sweep.period  = (value >> 4) & 0x7;
        pulse2.sweep.negate  = value & (1 << 3);
        pulse2.sweep.shift   = value & 0x7;
        pulse2.sweep.reload  = true;
        break;
    case APU_SQ2_LO:
        pulse2.period = (pulse2.period & 0xf0) | (value << 0);
        break;
    case APU_SQ2_HI:
        pulse2.period = (pulse2.period & 0x0f) | (value << 4);
        pulse2.reload_period();
        break;

    case APU_TRI_LINEAR:
        break;
    case APU_TRI_LO:
        break;
    case APU_TRI_HI:
        break;
    case APU_NOISE_VOL:
        break;
    case APU_NOISE_LO:
        break;
    case APU_NOISE_HI:
        break;
    case APU_DMC_FREQ:
        break;
    case APU_DMC_RAW:
        break;
    case APU_DMC_START:
        break;
    case APU_DMC_LEN:
        break;
    case APU_CONTROL:
        pulse1.length_counter.set_enable(value & 0x1);
        pulse2.length_counter.set_enable(value & 0x2);
        break;
    case APU_FRAME_CONTROL:
        frame_counter.mode              = static_cast<FrameCounter::Mode>(value >> 7);
        frame_counter.interrupt_inhibit = value >> 6;
        // TODO:
        break;
    }
}

Byte APU::readStatus()
{
    return (pulse1.length_counter.is_enabled() << 0 | pulse2.length_counter.is_enabled() << 1);
}

FrameCounter APU::setup_frame_counter(IRQ& irq)
{
    return FrameCounter(
      {
        std::ref(pulse1.volume),
        std::ref(pulse1.sweep),
        std::ref(pulse1.length_counter),
        std::ref(pulse2.volume),
        std::ref(pulse2.sweep),
        std::ref(pulse2.length_counter),
      },
      irq);
}
}
