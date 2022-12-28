#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <portaudio.h>
#include <atomic>
#include "spsc.hpp"

using SampleType = float;

class AudioQueue {

public:
    AudioQueue(int sample_rate, int buffer_size);
    ~AudioQueue();

    bool start();
    bool stop();

    bool push_sample(SampleType sample) {
        return buffer.push(sample);
    }

    std::size_t bufferSize() {
        return buffer.size();
    }

private:
    int audioCallback(
        const void *inputBUffer,
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags);

    PaStreamParameters m_outputParameters;
    PaStream *m_stream;
    int m_sample_rate;

    spsc::RingBuffer<SampleType> buffer;
};
