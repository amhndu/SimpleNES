#pragma once

#include <chrono>
#include <miniaudio.h>

#include "APU/spsc.hpp"

namespace sn
{

const std::chrono::milliseconds callback_period_ms { 120 };

struct CallbackData
{
    spsc::RingBuffer<float>& ring_buffer;
    ma_resampler*            resampler;
    std::vector<float>       input_frames_buffer;
    bool                     mute;
    int                      remaining_buffer_rounds;
};

// Receives input at a fixed sample rate from the audio queue and uses miniaudio to resample and output to audiodevice
//
// Why not SFML? SFML's SoundStream introduces additional buffers and has it's own polling mechanism which introduces
// extra lag. Effectively using it would mean relying on it's implementation-specific behaviour Using miniaudio is
// simpler as we just need to implement one audio callback
class AudioPlayer
{
public:
    const int output_sample_rate = ma_standard_sample_rate_44100;

    AudioPlayer(int input_rate)
      : input_sample_rate(input_rate)
      , audio_queue(4 * input_rate *
                    (callback_period_ms.count() / 100)) // big enough to keep 4 callback's worth of samples
      , cb_data { audio_queue, &resampler, {}, false, 1 }
    {
    }
    ~AudioPlayer();

    bool                    start();
    void                    mute();

    const int               input_sample_rate;
    // ONLY safe for 1 writer and 1 reader
    spsc::RingBuffer<float> audio_queue;

private:
    CallbackData     cb_data;

    bool             initialized = false;
    ma_device_config deviceConfig;
    ma_device        device;
    ma_resampler     resampler;
};
}
