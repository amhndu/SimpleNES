#include "APU/FrameCounter.h"

namespace sn
{

void FrameCounter::clearFrameInterrupt()
{
    if (frame_interrupt)
    {
        frame_interrupt = false;
        irq.release();
    }
};

void FrameCounter::reset(Mode m, bool irq_inhibit)
{
    mode              = m;
    interrupt_inhibit = irq_inhibit;
    // TODO: delay reset by 3-4 cycles?
    counter           = 0;
    if (interrupt_inhibit)
    {
        clearFrameInterrupt();
    }
    if (mode == Seq5Step)
    {
        for (FrameClockable& c : frame_slots)
        {
            // clock envelopes & triangle's linear counter
            c.quarter_frame_clock();
            // clock length counter & sweep units
            c.half_frame_clock();
        }
    }
}

void FrameCounter::clock()
{
    counter += 1;

    switch (counter)
    {
    case Q1:
        for (FrameClockable& c : frame_slots)
        {
            // clock envelopes & triangle's linear counter
            c.quarter_frame_clock();
        }
        break;
    case Q2:
        for (FrameClockable& c : frame_slots)
        {
            // clock envelopes & triangle's linear counter
            c.quarter_frame_clock();
            // clock length counter & sweep units
            c.half_frame_clock();
        }
        break;
    case Q3:
        for (FrameClockable& c : frame_slots)
        {
            // clock envelopes & triangle's linear counter
            c.quarter_frame_clock();
        }
        break;
    // case preQ4:
    //     // only 4-step
    //     if (mode != Seq4Step) {
    //         break;
    //     }
    //     // set frame irq if not inhibit
    //     if (!interrupt_inhibit && mode == Seq4Step) {
    //         irq();
    //     }
    //     break;
    case Q4:
        // only 4-step
        if (mode != Seq4Step)
        {
            break;
        }
        for (FrameClockable& c : frame_slots)
        {
            // clock envelopes & triangle's linear counter
            c.quarter_frame_clock();
            // clock length counter & sweep units
            c.half_frame_clock();
        }
        // set frame irq if not inhibit
        if (!interrupt_inhibit && mode == Seq4Step)
        {
            irq.pull();
            frame_interrupt = true;
        }
        break;
    // case postQ4:
    //     // only 4-step
    //     if (mode != Seq4Step) {
    //         break;
    //     }
    //     // set frame irq if not inhibit
    //     if (!interrupt_inhibit && mode == Seq4Step) {
    //         irq();
    //     }
    //     break;
    case Q5:
        // only 5-step
        if (mode != Seq5Step)
        {
            break;
        }
        for (FrameClockable& c : frame_slots)
        {
            // clock envelopes & triangle's linear counter
            c.quarter_frame_clock();
            // clock length counter & sweep units
            c.half_frame_clock();
        }
        break;
    };

    if ((mode == Seq4Step && counter == seq4step_length) || (/* mode == Seq5Step && */ counter == seq5step_length))
    {
        counter = 0;
    }
}
}
