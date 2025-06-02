#pragma once

#include <chrono>

namespace sn
{
using namespace std::chrono;

const float max_volume_f        = static_cast<float>(0xF);
const int   max_volume          = 0xF;

// NES CPU clock period
const auto  cpu_clock_period_ns = nanoseconds(559);
const auto  cpu_clock_period_s  = duration_cast<duration<double>>(cpu_clock_period_ns);
// The apu is clocked every second cpu period
const auto  apu_clock_period_ns = cpu_clock_period_ns * 2;
const auto  apu_clock_period_s  = duration_cast<duration<double>>(apu_clock_period_ns);

}
