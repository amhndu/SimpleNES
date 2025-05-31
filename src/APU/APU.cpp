#include "APU/APU.h"
#include "APU/Constants.h"
#include "APU/FrameCounter.h"
#include "APU/Pulse.h"
#include "APU/Timer.h"
#include "APU/spsc.hpp"
#include "Cartridge.h"
#include "Log.h"

#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>
#include <functional>
#include <ios>
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

    noise.clock();
    dmc.clock();
    triangle.clock();
    if (divideByTwo)
    {
        pulse1.clock();
        pulse2.clock();
    }
    divideByTwo = !divideByTwo;

    for (int clocks = sampling_timer.clock(cpu_clock_period_ns); clocks > 0; --clocks)
    {
        float pulse1out = static_cast<float>(pulse1.sample());
        float pulse2out = static_cast<float>(pulse2.sample());
        float pulse_out = 0;
        if (pulse1out + pulse2out != 0)
        {
            pulse_out = 95.88 / ((8128.0 / (pulse1out + pulse2out)) + 100.0);
        }

        float tnd_out     = 0;
        float triangleout = static_cast<float>(triangle.sample());
        float noiseout    = static_cast<float>(noise.sample());
        float dmcout      = static_cast<float>(dmc.sample());

        if (triangleout + noiseout + dmcout != 0)
        {
            float tnd_sum = (triangleout / 8227.0) + (noiseout / 12241.0) + (dmcout / 22638.0);
            tnd_out       = 159.79 / (1.0 / tnd_sum + 100.0);
        }
        float final = pulse_out + tnd_out;

        audio_queue.push(final);
    }
}

void APU::writeRegister(Address addr, Byte value)
{
    switch (addr)
    {
        break;

    case APU_SQ1_VOL:
        pulse1.volume.fixedVolumeOrPeriod = value & 0xf;
        pulse1.volume.constantVolume      = value & (1 << 4);
        pulse1.length_counter.halt        = value & (1 << 5);
        pulse1.seq_type                   = static_cast<Pulse::Duty::Type>(value >> 6);
        LOG(CpuTrace) << "APU_SQ1_VOL " << std::hex << +value << std::dec << std::boolalpha
                      << VAR_PRINT(pulse1.volume.fixedVolumeOrPeriod) << VAR_PRINT(pulse1.volume.constantVolume)
                      << VAR_PRINT(pulse1.length_counter.halt) << VAR_PRINT(int(pulse1.seq_type)) << std::endl;
        break;

    case APU_SQ1_SWEEP:
        pulse1.sweep.enabled = value & (1 << 7);
        pulse1.sweep.period  = (value >> 4) & 0x7;
        pulse1.sweep.negate  = value & (1 << 3);
        pulse1.sweep.shift   = value & 0x7;
        pulse1.sweep.reload  = true;
        LOG(CpuTrace) << "APU_SQ1_SWEEP " << std::hex << +value << std::dec << std::boolalpha
                      << VAR_PRINT(pulse1.sweep.enabled) << VAR_PRINT(pulse1.sweep.period)
                      << VAR_PRINT(pulse1.sweep.negate) << +VAR_PRINT(pulse1.sweep.shift) << std::endl;
        break;

    case APU_SQ1_LO:
    {
        int new_period = (pulse1.period & 0xff00) | value;
        LOG(CpuTrace) << "APU_SQ1_LO " << std::hex << +value << std::dec << std::endl;
        pulse1.set_period(new_period);
        break;
    }

    case APU_SQ1_HI:
    {
        int new_period = (pulse1.period & 0x00ff) | ((value & 0x7) << 8);
        pulse1.length_counter.set_from_table(value >> 3);
        pulse1.seq_idx = 0;
        pulse1.volume.divider.reset();
        pulse1.set_period(new_period);
        LOG(CpuTrace) << "APU_SQ1_HI " << std::hex << +value << std::dec << VAR_PRINT(pulse1.period)
                      << VAR_PRINT(pulse1.seq_idx) << VAR_PRINT(pulse1.length_counter.counter) << std::endl;
        break;
    }

    case APU_SQ2_VOL:
        pulse2.volume.fixedVolumeOrPeriod = value & 0xf;
        pulse2.volume.constantVolume      = value & (1 << 4);
        pulse2.length_counter.halt        = value & (1 << 5);
        pulse2.seq_type                   = static_cast<Pulse::Duty::Type>(value >> 6);
        LOG(CpuTrace) << "APU_SQ2_VOL" << std::hex << +value << std::dec << std::boolalpha
                      << VAR_PRINT(pulse2.volume.fixedVolumeOrPeriod) << VAR_PRINT(pulse2.volume.constantVolume)
                      << VAR_PRINT(pulse2.length_counter.halt) << VAR_PRINT(int(pulse2.seq_type)) << std::endl;
        break;

    case APU_SQ2_SWEEP:
        pulse2.sweep.enabled = value & (1 << 7);
        pulse2.sweep.period  = (value >> 4) & 0x7;
        pulse2.sweep.negate  = value & (1 << 3);
        pulse2.sweep.shift   = value & 0x7;
        pulse2.sweep.reload  = true;
        LOG(CpuTrace) << "APU_SQ2_SWEEP" << std::boolalpha << VAR_PRINT(pulse2.sweep.enabled)
                      << VAR_PRINT(pulse2.sweep.period) << VAR_PRINT(pulse2.sweep.negate)
                      << +VAR_PRINT(pulse2.sweep.shift) << std::endl;
        break;

    case APU_SQ2_LO:
    {
        int new_period = (pulse2.period & 0xff00) | value;
        LOG(CpuTrace) << "APU_SQ2_LO" << std::endl;
        pulse2.set_period(new_period);
        break;
    }

    case APU_SQ2_HI:
    {
        int new_period = (pulse2.period & 0x00ff) | ((value & 0x7) << 8);
        pulse2.length_counter.set_from_table(value >> 3);
        pulse2.seq_idx = 0;
        pulse2.set_period(new_period);
        pulse2.volume.divider.reset();
        LOG(CpuTrace) << "APU_SQ2_HI" << VAR_PRINT(pulse2.period) << VAR_PRINT(pulse2.seq_idx)
                      << VAR_PRINT(pulse2.length_counter.counter) << std::endl;
        break;
    }

    case APU_TRI_LINEAR:
        triangle.linear_counter.set_linear(value & 0x7f);
        triangle.linear_counter.reload  = true;
        // same bit is used for both the length counter half and linear counter control
        triangle.length_counter.halt    = 1 >> 7;
        triangle.linear_counter.control = 1 >> 7;
        LOG(CpuTrace) << "APU_TRI_LINEAR" << VAR_PRINT(triangle.linear_counter.reloadValue) << std::boolalpha
                      << VAR_PRINT(triangle.length_counter.halt) << VAR_PRINT(triangle.linear_counter.control)
                      << VAR_PRINT(triangle.length_counter.counter) << std::endl;
        break;

    case APU_TRI_LO:
    {
        int new_period = (triangle.period & 0xff00) | value;
        triangle.set_period(new_period);
        LOG(CpuTrace) << "APU_TRI_LOW" << std::endl;
        break;
    }

    case APU_TRI_HI:
    {
        int new_period = (triangle.period & 0x00ff) | ((value & 0x7) << 8);
        triangle.length_counter.set_from_table(value >> 3);
        triangle.seq_idx = 0;
        triangle.set_period(new_period);
        triangle.linear_counter.reload = true;
        LOG(CpuTrace) << "APU_SQ2_HI" << VAR_PRINT(triangle.period) << VAR_PRINT(triangle.seq_idx)
                      << VAR_PRINT(triangle.length_counter.counter) << VAR_PRINT(triangle.linear_counter.reloadValue)
                      << std::endl;
        break;
    }

    case APU_NOISE_VOL:
        noise.volume.fixedVolumeOrPeriod = value & 0xf;
        noise.volume.constantVolume      = value & (1 << 4);
        noise.length_counter.halt        = value & (1 << 5);
        LOG(CpuTrace) << "APU_NOISE_VOL" << std::boolalpha << VAR_PRINT(noise.volume.fixedVolumeOrPeriod)
                      << VAR_PRINT(noise.volume.constantVolume) << VAR_PRINT(noise.length_counter.halt)
                      << VAR_PRINT(int(noise.shift_register)) << std::endl;
        break;

    case APU_NOISE_LO:
        noise.mode = static_cast<Noise::Mode>(value & (1 << 7));
        noise.set_period_from_table(value & 0xf);
        LOG(CpuTrace) << "APU_NOISE_VOL" << std::boolalpha << VAR_PRINT(noise.mode) << VAR_PRINT(noise.period)
                      << std::endl;
        break;

    case APU_NOISE_HI:
        noise.length_counter.set_from_table(value >> 3);
        noise.volume.divider.reset();
        LOG(CpuTrace) << "APU_NOISE_VOL" << std::boolalpha << VAR_PRINT(noise.length_counter.counter) << std::endl;
        break;

    case APU_DMC_FREQ:
        dmc.irqEnable = value >> 7;
        dmc.loop      = value >> 6;
        dmc.set_rate(value & 0xf);
        LOG(CpuTrace) << "APU_DMC_FREQ" << std::boolalpha << VAR_PRINT(dmc.irqEnable) << VAR_PRINT(dmc.loop)
                      << VAR_PRINT(dmc.change_rate.get_period()) << std::endl;
        break;

    case APU_DMC_RAW:
        dmc.volume = value & 0x7f;
        LOG(CpuTrace) << "APU_DMC_RAW" << VAR_PRINT(+dmc.volume) << std::endl;
        break;

    case APU_DMC_START:
        dmc.sample_begin = 0xc000 | (value << 6);
        LOG(CpuTrace) << "APU_DMC_START" << VAR_PRINT(dmc.sample_begin) << std::endl;
        break;

    case APU_DMC_LEN:
        dmc.sample_length = (value << 4) | 1;
        LOG(CpuTrace) << "APU_DMC_LEN" << VAR_PRINT(dmc.sample_length) << std::endl;
        break;

    case APU_CONTROL:
        pulse1.length_counter.set_enable(value & 0x1);
        pulse2.length_counter.set_enable(value & 0x2);
        triangle.length_counter.set_enable(value & 0x4);
        noise.length_counter.set_enable(value & 0x8);
        dmc.control(value & 0x16);
        LOG(CpuTrace) << "APU_CONTROL" << std::boolalpha << VAR_PRINT(pulse1.length_counter.is_enabled())
                      << VAR_PRINT(pulse2.length_counter.is_enabled())
                      << VAR_PRINT(triangle.length_counter.is_enabled()) << VAR_PRINT(noise.length_counter.is_enabled())
                      << VAR_PRINT(dmc.change_enabled) << std::endl;
        break;

    case APU_FRAME_CONTROL:
        frame_counter.reset(static_cast<FrameCounter::Mode>(value >> 7), value >> 6);
        LOG(CpuTrace) << "APU_FRAME_CONTROL" << +VAR_PRINT(value) << std::endl;
        break;
    }
}

Byte APU::readStatus()
{
    bool last_frame_interrupt = frame_counter.frame_interrupt;
    frame_counter.clearFrameInterrupt();
    bool dmc_interrupt = dmc.interrupt;
    dmc.clear_interrupt();
    LOG(CpuTrace) << "APU_STATUS" << std::endl;
    return (pulse1.length_counter.is_enabled() << 0 | pulse2.length_counter.is_enabled() << 1 |
            triangle.length_counter.is_enabled() << 2 | noise.length_counter.is_enabled() << 3 |
            (dmc.remaining_bytes > 0) << 4 | last_frame_interrupt << 6 | dmc_interrupt << 7);
}

FrameCounter APU::setup_frame_counter(IRQHandle& irq)
{
    return FrameCounter(
      {
        std::ref(pulse1.volume),
        std::ref(pulse1.sweep),
        std::ref(pulse1.length_counter),

        std::ref(pulse2.volume),
        std::ref(pulse2.sweep),
        std::ref(pulse2.length_counter),

        std::ref(triangle.length_counter),

        std::ref(noise.length_counter),
        std::ref(noise.volume),
      },
      irq);
}
}
