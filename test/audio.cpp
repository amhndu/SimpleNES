#include "APU/spsc.hpp"
#include "Log.h"
#include "miniaudio.h"

#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <stdio.h>
#include <thread>
#include <vector>
#include <SFML/Audio/SoundStream.hpp>

using namespace std::chrono;

// REFERENCE: Downsampling:
// https://web.archive.org/web/20211206113800/https://forums.nesdev.org/viewtopic.php?t=8602

const auto clock_period_ns = nanoseconds(559);
const auto clock_period_s = duration_cast<duration<double>>(clock_period_ns);

const auto sample_rate_std = ma_standard_sample_rate_44100;

// NOT analgous to NES timers which have a period of (t+1) and count from t -> 0 instead of 1 -> t
struct Divider {
public:
    explicit Divider(int period) : period(period) {}
    int period;

    bool clock() {
        leftover += 1;
        if (leftover < period) {
            return false;
        }

        leftover = 0;
        return true;
    }
private:
  int leftover = 0;
};

struct Timer {
public:
    explicit Timer(nanoseconds period) : period(period) {}
    const nanoseconds period;

    // clock the timer and return number of periods elapsed
    int clock(nanoseconds elapsed) {
        leftover += elapsed;
        if (leftover < elapsed) {
            return 0;
        }

        auto cycles = leftover / period;
        leftover = leftover % period;
        return cycles;
    }
private:
    nanoseconds leftover = nanoseconds(0);
};

// Pulse generates output from a fixed clock and pushes to the output buffer
struct Pulse {
private:
    const std::vector<int> sequence = {0, 0, 0, 0, 1, 1, 1, 1};
    const float volume = 0.5;

    uint sequence_idx = 0;
    Divider sequence_stepper {0};

public: 
    void set_frequency(double output_freq) {
        // output_f = divider_freq / 8 = clock_freq / (8 * divider_p) = 1 / (8 * divider_p * clock_period)
        // divider_p = 1 / (8 * clock_period * output_f)
        auto period = 1.0 / (8.0 * clock_period_s.count() * output_freq);
        sequence_stepper.period = static_cast<int>(period);
        LOG(sn::Info) << "pulse t" << sequence_stepper.period << std::endl;
    }

    void clock() {
        if (sequence_stepper.clock()) {
            sequence_idx = (8 + (sequence_idx - 1)) % 8;
        }
    }

    float sample() const {
        if (sequence[sequence_idx] == 0) {
            return 0;
        }

        return volume;
    }
};

struct Mixer {
    const Pulse &pulse;
    spsc::RingBuffer<float> &audio_queue;

    Timer sampling_frequency {nanoseconds(int(1e9 / sample_rate_std))};

    void clock(nanoseconds elapsed) {
        for (int frames = sampling_frequency.clock(elapsed); frames > 0; --frames) {
            float frame = pulse.sample();
            audio_queue.push(frame);
        }
    }
};

void audio_generator(spsc::RingBuffer<float>& audio_queue) {
    using namespace std::chrono;

    auto last_wakeup = high_resolution_clock::now();

    Timer global_clock { clock_period_ns };
    Pulse pulser;
    pulser.set_frequency(80);
    Mixer mixer { pulser, audio_queue };

    while (true) {
        const auto wakeup_ts = high_resolution_clock::now();
        for (int clocks = global_clock.clock(wakeup_ts - last_wakeup); clocks > 0; --clocks) {
            pulser.clock();
            mixer.clock(global_clock.period);
        }

        // run every (1/60)s ≈ 16ms
        // audio callback occurs every ≈ 100ms
        // we should have 6 * callback worth of data saved up
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60)); 
        last_wakeup = wakeup_ts;
    }
}

#include <SFML/Audio.hpp>

class AudioStream : public sf::SoundStream {
    const int skipRounds = 3;
public:
    AudioStream(spsc::RingBuffer<float>& ringBuffer)
        : m_ringBuffer(ringBuffer), m_remainingBufferRounds(skipRounds) {
        setProcessingInterval(sf::milliseconds(5));
        initialize(1, sample_rate_std); // Mono audio with the desired sample rate
    }

protected:
    bool onGetData(Chunk& data) override {
        // SFML immediately calls 3 times to fill up all buffers, we offer 3 buffers 1/3rd the size of the usual one to minimize latency
        // leading to a total of 200ms total latency
        if (m_remainingBufferRounds-- > 0) {
            LOG(sn::Info) << "Skipping buffer round" << std::endl;
            m_silenceBuffer.assign(bufferSize / (skipRounds * 2.f), 0.f);
            data.samples = m_silenceBuffer.data();
            data.sampleCount = bufferSize;
            return true;
        }

        auto framesAvail = m_ringBuffer.pop(m_audioBufferFloat.data(), bufferSize);
        LOG(sn::Info) << "avail: " << framesAvail << ", required: " << bufferSize << std::endl;

        if (framesAvail < bufferSize) {
            std::cout << "Emitting zeroes: " << bufferSize - framesAvail << std::endl;
            for (int remaining = bufferSize - framesAvail; remaining > 0; --remaining) {
                m_audioBufferFloat[framesAvail + (bufferSize - remaining)] = 0.f;
            }
        }

        // Convert float buffer to int16 buffer
        for (size_t i = 0; i < bufferSize; ++i) {
            m_audioBufferInt16[i] = static_cast<sf::Int16>(std::clamp(m_audioBufferFloat[i] * 32767.f, -32768.f, 32767.f));
        }

        data.samples = m_audioBufferInt16.data();
        data.sampleCount = bufferSize;
        return true;
    }


    void onSeek(sf::Time timeOffset) override {
        // Seeking is not supported in this example
        std::cerr << "Seeking not supported" << std::endl;
    }

private:
    static constexpr size_t bufferSize = sample_rate_std / 10;

    spsc::RingBuffer<float>& m_ringBuffer;
    int m_remainingBufferRounds;

    std::vector<float> m_audioBufferFloat = std::vector<float>(bufferSize, 0.f);
    std::vector<sf::Int16> m_audioBufferInt16 = std::vector<sf::Int16>(bufferSize, 0);
    std::vector<sf::Int16> m_silenceBuffer;
};

int main() {
    sn::Log::get().setLogStream(std::cout);
    sn::Log::get().setLevel(sn::Info);

    spsc::RingBuffer<float> audio_queue(sample_rate_std);

    AudioStream audioStream(audio_queue, 3);
    audioStream.play();

    audio_generator(audio_queue);


    return 0;
}



/*struct CallbackData {*/
/*    spsc::RingBuffer<float>& ring_buffer;*/
/*    int remaining_buffer_rounds;*/
/*};*/
/**/
/*void data_callback(ma_device* pDevice, void* pOutput, [[maybe_unused]] const void* pInput, ma_uint32 frameCount)*/
/*{*/
/*    if (pDevice->pUserData == nullptr) {*/
/*        return;*/
/*    }*/
/**/
/*    CallbackData& cbData = *(CallbackData*)pDevice->pUserData;*/
/**/
/*    if (cbData.remaining_buffer_rounds-- > 0) {*/
/*        LOG(sn::InfoVerbose) << "skipping buffer round" << std::endl;*/
/*        return;*/
/*    }*/
/**/
/*    auto framesAvail = cbData.ring_buffer.pop(reinterpret_cast<float*>(pOutput), frameCount);*/
/*    printf("avail: %zu, required: %d\n", framesAvail, frameCount);*/
/**/
/*    if (framesAvail < frameCount) {*/
/*        printf("emitting zereoes: %zu\n", frameCount - framesAvail);*/
/*        float *fOutput = reinterpret_cast<float*>(pOutput);*/
/*        for (int remaining = frameCount - framesAvail; remaining > 0; --remaining) {*/
/*            fOutput[remaining + framesAvail] = 0;*/
/*        }*/
/*    }*/
/*}*/
/**/
/*int main()*/
/*{*/
/*    sn::Log::get().setLogStream(std::cout);*/
/*    sn::Log::get().setLevel(sn::InfoVerbose);*/
/**/
/*    spsc::RingBuffer<float> audio_queue { sample_rate_std };*/
/**/
/*    CallbackData cbData {*/
/*        audio_queue,*/
/*        3,*/
/*    };*/
/**/
/*    ma_device_config deviceConfig;*/
/*    ma_device device;*/
/**/
/*    deviceConfig = ma_device_config_init(ma_device_type_playback);*/
/*    deviceConfig.playback.format   = ma_format_f32; // TODO: experiment with ints*/
/*    deviceConfig.playback.channels = 1;*/
/*    deviceConfig.sampleRate        = sample_rate_std;*/
/*    deviceConfig.dataCallback      = data_callback;*/
/*    deviceConfig.pUserData         = &cbData;*/
/*    deviceConfig.periodSizeInMilliseconds = 100;*/
/**/
/*    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {*/
/*        printf("Failed to open playback device.\n");*/
/*        return -3;*/
/*    }*/
/**/
/*    if (ma_device_start(&device) != MA_SUCCESS) {*/
/*        printf("Failed to start playback device.\n");*/
/*        ma_device_uninit(&device);*/
/*        return -4;*/
/*    }*/
/**/
/*    audio_generator(audio_queue);*/
/**/
/*    ma_device_uninit(&device);*/
/*    return 0;*/
/*}*/
