#pragma once

#include "IRQ.h"
#include <functional>
#include <vector>

namespace sn
{
struct FrameClockable
{
    // will be called every quarter frame (including half frames)
    virtual void quarter_frame_clock() {};
    // will be called every half frame
    virtual void half_frame_clock() {};
};

struct FrameCounter
{
    constexpr static int                                Q1              = 7457;
    constexpr static int                                Q2              = 14913;
    constexpr static int                                Q3              = 22371;
    constexpr static int                                preQ4           = 29828;
    constexpr static int                                Q4              = 29829;
    constexpr static int                                postQ4          = 29830;
    constexpr static int                                seq4step_length = 29830;

    constexpr static int                                Q5              = 37281;
    constexpr static int                                seq5step_length = 29830;

    std::vector<std::reference_wrapper<FrameClockable>> frame_slots;

    enum Mode
    {
        Seq4Step = 0,
        Seq5Step = 1,
    } mode                 = Seq4Step;
    int  counter           = 0;
    bool interrupt_inhibit = false;

    IRQ& irq;

    FrameCounter(std::vector<std::reference_wrapper<FrameClockable>> slots, IRQ& irq)
      : frame_slots(slots)
      , irq(irq)
    {
    }

    void clock();
};
}
