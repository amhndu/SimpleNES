#include "APU/FrameCounter.h"
#include "Log.h"

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

// clocked at apu freq (half the cpu freq)
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
        LOG(CpuTrace) << "framecounter: Q1 clock" << std::endl;
        break;
    case Q2:
        for (FrameClockable& c : frame_slots)
        {
            // clock envelopes & triangle's linear counter
            c.quarter_frame_clock();
            // clock length counter & sweep units
            c.half_frame_clock();
        }
        LOG(CpuTrace) << "framecounter: Q2 clock" << std::endl;
        break;
    case Q3:
        for (FrameClockable& c : frame_slots)
        {
            // clock envelopes & triangle's linear counter
            c.quarter_frame_clock();
        }
        LOG(CpuTrace) << "framecounter: Q3 clock" << std::endl;
        break;
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
        LOG(CpuTrace) << "framecounter: Q4 clock" << std::endl;
        // set frame irq if not inhibit
        if (!interrupt_inhibit)
        {
            irq.pull();
            frame_interrupt = true;
        }
        break;
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
        LOG(CpuTrace) << "framecounter: Q5 clock" << std::endl;
        break;
    };

    if ((mode == Seq4Step && counter == seq4step_length) || (/* mode == Seq5Step && */ counter == seq5step_length))
    {
        counter = 0;
    }
}
}
