#pragma once

#include <miniaudio.h>

#include "APU/spsc.hpp"
#include "Log.h"

namespace sn {

    struct CallbackData {
        spsc::RingBuffer<float>& ring_buffer;
        int remaining_buffer_rounds;
    };

    // AudioPlayer uses miniaudio to play the samples directly
    // SFML's SoundStream introduces additional buffers and has it's own polling mechanism which introduces extra lag. Effectively using it would mean relying on it's implementation-specific behaviour
    // Using miniaudio is simpler as we just need to implement one audio callback
    class AudioPlayer {
    public:
        const int std_sample_rate = ma_standard_sample_rate_44100;

        AudioPlayer(): cb_data {audio_queue, 3} {
            LOG(Info) << "AudioPlayer ptr" << this << std::endl;
            LOG(Info) << "AudioPlayer std_sample_rate" << std_sample_rate << "Hz" << std::endl;
        }
        ~AudioPlayer();

        bool start();

        // ONLY safe for 1 writer and 1 reader
        spsc::RingBuffer<float> audio_queue { 2*std_sample_rate };
    private:
        CallbackData cb_data;

        bool initialized = false;
        ma_device_config deviceConfig;
        ma_device device;
    };
}
