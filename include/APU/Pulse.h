#pragma once

#include <cstddef>
#include <functional>
#include <optional>

#include "APU/Constants.h"
#include "APU/Divider.h"
#include "APU/FrameCounter.h"
#include "Cartridge.h"
#include "IRQ.h"

namespace sn
{

// Acts as either length counter or linear counter.
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
    void    quarter_frame_clock() override;

    int     get() const;

    Divider divider { 0 };
    uint    fixedVolumeOrPeriod = max_volume;
    uint    decayVolume         = max_volume;
    bool    constantVolume      = true;
    bool    isLooping           = false;
    bool    shouldStart         = false;
};

struct Pulse
{
    struct Duty
    {
        enum class Type
        {
            SEQ_12_5   = 0,
            SEQ_25     = 1,
            SEQ_50     = 2,
            SEQ_25_INV = 3,
        };
        static const int   Count  = 4;
        static const int   Length = 8;

        static inline bool active(Type cycle, int idx)
        {
            const bool _sequences[] {
                0, 0, 0, 0, 0, 0, 0, 1, // 12.5%
                0, 0, 0, 0, 0, 0, 1, 1, //   25%
                0, 0, 0, 0, 1, 1, 1, 1, //   50%
                1, 1, 1, 1, 1, 1, 0, 0, //   25% negated
            };
            return _sequences[static_cast<int>(cycle) * Length + idx];
        }
    };

    Volume        volume;
    LengthCounter length_counter;

    uint          seq_idx { 0 };
    Duty::Type    seq_type { Duty::Type::SEQ_50 };
    Divider       sequencer { 0 };
    int           period = 0;

    enum class Type
    {
        Pulse1 = 1,
        Pulse2 = 2,
    } type;

    Pulse(Type type)
      : type(type)
      , sweep(*this, type == Type::Pulse1)
    {
    }

    struct Sweep : public FrameClockable
    {
        Pulse&   pulse;

        int      period          = 0;
        bool     enabled         = false;
        bool     reload          = false;
        bool     negate          = false;
        sn::Byte shift           = 0;
        bool     ones_complement = false;

        Divider  divider { 0 };

        Sweep(Pulse& pulse, bool ones_complement)
          : pulse(pulse)
          , ones_complement(ones_complement)
        {
        }

        void        half_frame_clock() override;

        static bool is_muted(int current, int target) { return current < 8 || target > 0x7FF; }

        int         calculate_target(int current) const;

    } sweep;

    void set_period(int p);

    // Clocked at half the cpu freq
    void clock();

    Byte sample() const;
};

struct Triangle
{
    LengthCounter length_counter;
    LinearCounter linear_counter;

    uint          seq_idx { 0 };
    Divider       sequencer { 0 };
    int           period = 0;

    void          set_period(int p);

    // Clocked at the cpu freq
    void          clock();

    Byte          sample() const;

    int           volume() const;
};

struct Noise
{
    Volume        volume;
    LengthCounter length_counter;
    Divider       divider { 0 };

    enum Mode : bool
    {
        Bit1 = 0,
        BIt6 = 1,
    } mode              = Bit1;
    int  period         = 0;
    int  shift_register = 1;

    void set_period_from_table(int idx);

    // Clocked at the cpu freq
    void clock();

    Byte sample() const;
};

struct DMC
{
    bool    irqEnable      = false;
    bool    loop           = false;

    int     volume         = 0;

    bool    change_enabled = false;
    Divider change_rate { 0 };

    Address sample_begin    = 0;
    int     sample_length   = 0;

    int     remaining_bytes = 0;
    Address current_address = 0;

    Byte    sample_buffer   = 0;

    int     shifter         = 0;
    int     remaining_bits  = 0;
    bool    silenced        = false;

    bool    interrupt       = false;

    void    set_irq_enable(bool enable);
    void    set_rate(int idx);
    void    control(bool enable);
    void    clear_interrupt();

    DMC(IRQHandle& irq, std::function<Byte(Address)> dma)
      : irq(irq)
      , dma(dma)
    {
    }

    // Clocked at the cpu freq
    void clock();

    Byte sample() const;

private:
    // Load sample and return if it was succesfully loaded
    bool                         load_sample();
    int                          pop_delta();
    IRQHandle&                   irq;
    std::function<Byte(Address)> dma;
};

}
