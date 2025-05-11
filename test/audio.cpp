#include "APU/spsc.hpp"
#include "Cartridge.h"
#include "Log.h"
#include "miniaudio.h"

#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <stdio.h>
#include <thread>
#include <utility>

using namespace std::chrono;

// REFERENCE: Downsampling:
// https://web.archive.org/web/20211206113800/https://forums.nesdev.org/viewtopic.php?t=8602

// The apu is clocked every second cpu period
const auto  cpu_clock_period_ns = nanoseconds(559);
const auto  cpu_clock_period_s  = duration_cast<duration<double>>(cpu_clock_period_ns);
// NES CPU clock period
const auto  apu_clock_period_ns = cpu_clock_period_ns * 2;
const auto  apu_clock_period_s  = duration_cast<duration<double>>(apu_clock_period_ns);

const auto  sample_rate_std     = ma_standard_sample_rate_44100;

const float max_volume_f        = static_cast<float>(0xF);
const int   max_volume          = 0xF;

struct FrameClockable
{
    // will be called every quarter frame (including half frames)
    virtual void quarter_frame_clock() {};
    // will be called every half frame
    virtual void half_frame_clock() {};
};

// Modeled after NES timers; which have a period of (t+1) and count from t -> 0 -> t -> ...
struct Divider
{
public:
    explicit Divider(int period)
      : period(period)
    {
    }

    bool clock()
    {
        if (counter == 0)
        {
            counter = period;
            return true;
        }

        counter -= 1;
        return false;
    }

    void reset(int p) { counter = period = p; }

    void reset() { counter = period; }

    int  get_period() const { return period; }

private:
    int period  = 0;
    int counter = 0;
};

struct Timer
{
public:
    explicit Timer(nanoseconds period)
      : period(period)
    {
    }
    const nanoseconds period;

    // clock the timer and return number of periods elapsed
    int               clock(nanoseconds elapsed)
    {
        leftover += elapsed;
        if (leftover < elapsed)
        {
            return 0;
        }

        auto cycles = leftover / period;
        leftover    = leftover % period;
        return cycles;
    }

private:
    nanoseconds leftover = nanoseconds(0);
};

struct LengthCounter : public FrameClockable
{
    constexpr static int length_table[] = {
        10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
        12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
    };

    void set_enable(bool new_value)
    {
        enabled = new_value;

        if (!enabled)
        {
            counter = 0;
        }
    }

    void set_counter(int new_value)
    {
        if (!enabled)
        {
            return;
        }

        counter = new_value;
    }

    void half_frame_clock() override
    {
        if (halt)
        {
            return;
        }

        if (counter == 0)
        {
            return;
        }

        --counter;
    }

    bool muted() const { return enabled && counter == 0; }

private:
    bool enabled = false;
    bool halt    = false;
    int  counter = 0;
};

struct EnvelopeGenerator : public FrameClockable
{
    void quarter_frame_clock() override
    {
        if (shouldStart)
        {
            shouldStart = false;
            decayVolume = max_volume;
            divider.reset(fixedVolumeOrPeriod);
            return;
        }

        if (!divider.clock())
        {
            return;
        }

        if (decayVolume > 0)
        {
            --decayVolume;
        }
        else if (isLooping)
        {
            decayVolume = max_volume;
        }
    }

    int get() const
    {
        if (constantVolume)
        {
            return fixedVolumeOrPeriod;
        }

        return decayVolume;
    }

    Divider divider { 0 };
    uint    fixedVolumeOrPeriod = max_volume;
    uint    decayVolume         = max_volume;
    bool    constantVolume      = true;
    bool    isLooping           = false;
    bool    shouldStart         = false;
};

// Pulse generates output from a fixed clock and pushes to the output buffer
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
        static constexpr int  Count  = 4;
        static constexpr int  Length = 8;
        static constexpr bool _sequences[] {
            0, 0, 0, 0, 0, 0, 0, 1, // 12.5%
            0, 0, 0, 0, 0, 0, 1, 1, //   25%
            0, 0, 0, 0, 1, 1, 1, 1, //   50%
            1, 1, 1, 1, 1, 1, 0, 0, //   25% negated
        };
        static constexpr inline bool active(Type cycle, int idx)
        {
            return _sequences[static_cast<int>(cycle) * Length + idx];
        }
    };

    EnvelopeGenerator envelope;
    LengthCounter     length_counter;

    bool              enabled     = false;
    bool              sweep_muted = false;

    int               period      = 0;
    uint              seq_idx { 0 };
    Duty::Type        seq_type { Duty::Type::SEQ_50 };
    Divider           sequencer { 0 };

    struct Sweep : public FrameClockable
    {
        Pulse&   pulse;

        Divider  divider { 0 };
        bool     enabled         = false;
        bool     reload          = false;
        bool     negate          = false;
        sn::Byte shift           = 0;
        bool     ones_complement = false;

        Sweep(Pulse& pulse)
          : pulse(pulse)
        {
        }

        void half_frame_clock() override
        {
            if (reload)
            {
                divider.reset();
                reload = false;
                return;
            }

            if (!divider.clock())
            {
                return;
            }

            if (enabled && shift > 0)
            {
                if (!pulse.sweep_muted)
                {
                    const auto current = pulse.sequencer.get_period();
                    auto       target  = calculate_target(current);
                    pulse.period       = target;
                    pulse.reload_period();
                }
            }
        };

        static bool is_muted(int current, int target) { return current < 8 || target > 0x7FF; }

        int         calculate_target(int current) const
        {
            const auto amt = current >> shift;
            if (!negate)
            {
                return current + amt;
            }

            if (ones_complement)
            {
                return std::max(0, current - amt);
            }

            return std::max(0, current - amt - 1);
        }

    } sweep;

    Pulse()
      : sweep(*this)
    {
    }

    void reload_period()
    {
        sequencer.reset(period);
        auto target = sweep.calculate_target(period);
        sweep_muted = sweep.is_muted(period, target);
    }

    void set_frequency(double output_freq)
    {
        // output_f = divider_freq / 8 = clock_freq / (8 * divider_p) = 1 / (8 * divider_p * clock_period)
        // divider_p = 1 / (8 * clock_period * output_f)
        auto period = 1.0 / (Duty::Length * apu_clock_period_s.count() * output_freq);
        sequencer.reset(static_cast<int>(period));
        LOG(sn::Info) << "pulse t: " << sequencer.get_period() << std::endl;
    }

    // Clocked at APU freq (ie. half the cpu)
    void apu_clock()
    {
        if (sequencer.clock())
        {
            // NES counts downwards in sequencer
            seq_idx = (8 + (seq_idx - 1)) % 8;
        }
    }

    sn::Byte sample() const
    {
        if (length_counter.muted())
        {
            return 0;
        }

        if (sweep_muted)
        {
            return 0;
        }

        if (!Duty::active(seq_type, seq_idx))
        {
            return 0;
        }

        return envelope.get();
    }
};

struct FrameCounter
{
    constexpr static int                                Seq4StepCpuCycles[] = { 7457, 14913, 22371, 29829 };
    constexpr static int                                Q1                  = 7457;
    constexpr static int                                Q2                  = 14913;
    constexpr static int                                Q3                  = 22371;
    constexpr static int                                preQ4               = 29828;
    constexpr static int                                Q4                  = 29829;
    constexpr static int                                postQ4              = 29830;
    constexpr static int                                seq4step_length     = 29830;

    constexpr static int                                Q5                  = 37281;
    constexpr static int                                seq5step_length     = 29830;

    std::vector<std::reference_wrapper<FrameClockable>> frame_slots;

    enum Mode
    {
        Seq4Step = 0,
        Seq5Step = 1,
    } mode                = Seq4Step;
    int  counter          = 0;
    bool interruptInhibit = false;

    void cpu_clock()
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
        case preQ4:
            // only 4-step
            if (mode != Seq4Step)
            {
                break;
            }
            // set frame irq if not inhibit
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
            // set frame irq if not inhibit
            break;
        case postQ4:
            // only 4-step
            if (mode != Seq4Step)
            {
                break;
            }
            // set frame irq if not inhibit
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
            break;
        };

        if ((mode == Seq4Step && counter == seq4step_length) || (/* mode == Seq5Step && */ counter == seq5step_length))
        {
            counter = 0;
        }
    }
};

struct APU
{
    Pulse        pulser;
    FrameCounter frame_counter;

    APU()
      : frame_counter(wire_frame_counter())
    {
    }

    void apu_step() { pulser.apu_clock(); }

    void cpu_step() { frame_counter.cpu_clock(); }

private:
    FrameCounter wire_frame_counter()
    {
        return FrameCounter {
            { std::ref(pulser.envelope), std::ref(pulser.sweep), std::ref(pulser.length_counter) },
        };
    }
};

struct Mixer
{
    const Pulse&             pulse;
    spsc::RingBuffer<float>& audio_queue;

    Timer                    sampling_frequency { nanoseconds(int(1e9 / sample_rate_std)) };

    void                     clock(nanoseconds elapsed)
    {
        for (int frames = sampling_frequency.clock(elapsed); frames > 0; --frames)
        {
            float frame = static_cast<float>(pulse.sample()) / max_volume_f;
            audio_queue.push(frame);
        }
    }
};

void audio_generator(spsc::RingBuffer<float>& audio_queue)
{
    using namespace std::chrono;

    auto  last_wakeup = high_resolution_clock::now();

    Timer apu_clock { apu_clock_period_ns };

    APU   apu;
    // apu.pulser.envelope.constantVolume      = false;
    // apu.pulser.envelope.isLooping           = true;
    // apu.pulser.envelope.shouldStart         = true;
    // apu.pulser.envelope.fixedVolumeOrPeriod = 1;
    apu.pulser.period = 768;
    apu.pulser.reload_period();
    apu.pulser.envelope.constantVolume      = true;
    apu.pulser.envelope.fixedVolumeOrPeriod = 4;
    apu.pulser.envelope.isLooping           = false;
    apu.pulser.envelope.shouldStart         = false;

    Mixer mixer { apu.pulser, audio_queue };

    LOG(sn::Info) << "timer started: " << apu_clock.period.count() << "ns" << std::endl;
    while (true)
    {
        const auto wakeup_ts = high_resolution_clock::now();
        for (int clocks = apu_clock.clock(wakeup_ts - last_wakeup); clocks > 0; --clocks)
        {
            apu.apu_step();
            mixer.clock(apu_clock.period);

            // 1 apu step = 2 cpu step = 2 * 3 ppu steps
            apu.cpu_step();

            apu.cpu_step();
        }

        // run every (1/60)s ≈ 16ms
        // audio callback occurs every ≈ 100ms
        // we should have 6 * callback worth of data saved up
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
        last_wakeup = wakeup_ts;
    }
}

/*
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
        // SFML immediately calls 3 times to fill up all buffers, we offer 3 buffers 1/3rd the size of the usual one to
minimize latency
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
            m_audioBufferInt16[i] = static_cast<sf::Int16>(std::clamp(m_audioBufferFloat[i] * 32767.f, -32768.f,
32767.f));
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
*/

struct CallbackData
{
    spsc::RingBuffer<float>& ring_buffer;
    int                      remaining_buffer_rounds;
};

void data_callback(ma_device* pDevice, void* pOutput, [[maybe_unused]] const void* pInput, ma_uint32 frameCount)
{
    if (pDevice->pUserData == nullptr)
    {
        return;
    }

    CallbackData& cbData = *(CallbackData*)pDevice->pUserData;

    if (cbData.remaining_buffer_rounds-- > 0)
    {
        LOG(sn::InfoVerbose) << "skipping buffer round" << std::endl;
        return;
    }

    auto framesAvail = cbData.ring_buffer.pop(reinterpret_cast<float*>(pOutput), frameCount);

    if (framesAvail < frameCount)
    {
        LOG(sn::InfoVerbose) << "not enoughd data; emitting zereoes: " << framesAvail << "; frameCount: " << frameCount
                             << std::endl;
        float* fOutput = reinterpret_cast<float*>(pOutput);
        for (int remaining = frameCount - framesAvail; remaining > 0; --remaining)
        {
            fOutput[remaining + framesAvail] = 0;
        }
    }
}

int main()
{
    sn::Log::get().setLogStream(std::cout);
    sn::Log::get().setLevel(sn::InfoVerbose);

    spsc::RingBuffer<float> audio_queue { sample_rate_std };

    CallbackData            cbData {
        audio_queue,
        3,
    };

    ma_device_config deviceConfig;
    ma_device        device;

    deviceConfig                          = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format          = ma_format_f32; // TODO: experiment with ints
    deviceConfig.playback.channels        = 1;
    deviceConfig.sampleRate               = sample_rate_std;
    deviceConfig.dataCallback             = data_callback;
    deviceConfig.pUserData                = &cbData;
    deviceConfig.periodSizeInMilliseconds = 100;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
    {
        printf("Failed to open playback device.\n");
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS)
    {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -4;
    }

    audio_generator(audio_queue);

    ma_device_uninit(&device);
    return 0;
}
