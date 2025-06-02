#pragma once

#include "APU/Divider.h"
#include "APU/FrameCounter.h"
#include "Cartridge.h"

namespace sn
{

struct Pulse;

struct PulseDuty
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
};

}
