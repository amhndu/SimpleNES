#include "APU/APU.h"
#include "APU/FrameCounter.h"
#include "APU/Pulse.h"
#include "APU/spsc.hpp"
#include "Cartridge.h"
#include "Log.h"

#include <SFML/Audio/SoundStream.hpp>
#include <SFML/Config.hpp>
#include <SFML/System/Time.hpp>
#include <ios>
#include <thread>  // 新增：用于空队列时的短暂等待
#include <vector>  // 新增：用于音频缓冲区

using namespace std::chrono;

namespace sn
{
// 新增：音频参数常量
constexpr unsigned int APU_SAMPLE_RATE = 44100;
constexpr size_t AUDIO_BUFFER_SIZE = 1024;

enum Register
{
    // 保持原有枚举定义不变...
    APU_SQ1_VOL       = 0x4000,
    APU_SQ1_SWEEP     = 0x4001,
    APU_SQ1_LO        = 0x4002,
    APU_SQ1_HI        = 0x4003,
    APU_SQ2_VOL       = 0x4004,
    APU_SQ2_SWEEP     = 0x4005,
    APU_SQ2_LO        = 0x4006,
    APU_SQ2_HI        = 0x4007,
    APU_TRI_LINEAR    = 0x4008,
    APU_TRI_LO        = 0x400a,
    APU_TRI_HI        = 0x400b,
    APU_NOISE_VOL     = 0x400c,
    APU_NOISE_LO      = 0x400e,
    APU_NOISE_HI      = 0x400f,
    APU_DMC_FREQ      = 0x4010,
    APU_DMC_RAW       = 0x4011,
    APU_DMC_START     = 0x4012,
    APU_DMC_LEN       = 0x4013,
    APU_CONTROL       = 0x4015,
    APU_FRAME_CONTROL = 0x4017,
};

// 保持mix函数不变
float mix(Byte pulse1, Byte pulse2, Byte triangle, Byte noise, Byte dmc)
{
    float pulse1out = static_cast<float>(pulse1); // 0-15
    float pulse2out = static_cast<float>(pulse2); // 0-15

    float pulse_out = 0;
    if (pulse1out + pulse2out != 0)
    {
        pulse_out = 95.88 / ((8128.0 / (pulse1out + pulse2out)) + 100.0);
    }

    float tnd_out     = 0;
    float triangleout = static_cast<float>(triangle); // 0-15
    float noiseout    = static_cast<float>(noise);    // 0-15
    float dmcout      = static_cast<float>(dmc);      // 0-127

    if (triangleout + noiseout + dmcout != 0)
    {
        float tnd_sum = (triangleout / 8227.0) + (noiseout / 12241.0) + (dmcout / 22638.0);
        tnd_out       = 159.79 / (1.0 / tnd_sum + 100.0);
    }

    return pulse_out + tnd_out;
}

// 新增：实现SFML音频流接口
bool APU::onGetData(sf::SoundStream::Chunk& data)
{
    static std::vector<sf::Int16> audioBuffer(AUDIO_BUFFER_SIZE);
    size_t samplesWritten = 0;

    // 从队列读取样本并转换格式
    while (samplesWritten < AUDIO_BUFFER_SIZE)
    {
        float sample;
        if (audio_queue.try_pop(sample))
        {
            // 转换混合后的float样本到16位整数（范围映射）
            // mix()输出约为0~150，映射到-32768~32767
            const float scale = 32767.0f / 150.0f;
            audioBuffer[samplesWritten++] = static_cast<sf::Int16>(sample * scale);
        }
        else
        {
            // 队列空时填充静音样本
            audioBuffer[samplesWritten++] = 0;
            // 短暂等待避免CPU占用过高
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }

    data.samples = audioBuffer.data();
    data.sampleCount = AUDIO_BUFFER_SIZE;
    return true;
}

// 新增：实现SFML音频流定位接口（NES APU无需此功能）
void APU::onSeek(sf::Time timeOffset)
{
    // 空实现，忽略定位请求
}

// 保持step函数不变（已正确将样本推入队列）
void APU::step()
{
    noise.clock();
    dmc.clock();
    triangle.clock();
    if (divideByTwo)
    {
        frame_counter.clock();
        pulse1.clock();
        pulse2.clock();

        audio_queue.push(mix(pulse1.sample(), pulse2.sample(), triangle.sample(), noise.sample(), dmc.sample()));
    }
    divideByTwo = !divideByTwo;
}

// 修改writeRegister函数：修复三角波寄存器错误
void APU::writeRegister(Address addr, Byte value)
{
    switch (addr)
    {
        // 保持其他寄存器处理不变...

    case APU_TRI_LINEAR:
        triangle.linear_counter.set_linear(value & 0x7f);
        triangle.linear_counter.reload  = true;
        // 修复：使用value的第7位（原代码1 >>7恒为0）
        triangle.linear_counter.control = triangle.length_counter.halt = (value >> 7) & 1;
        LOG(CpuTrace) << "APU_TRI_LINEAR " << std::hex << +value << std::dec
                      << VAR_PRINT(triangle.linear_counter.reloadValue) << std::boolalpha
                      << VAR_PRINT(triangle.length_counter.halt) << VAR_PRINT(triangle.linear_counter.control)
                      << VAR_PRINT(triangle.length_counter.counter) << std::endl;
        break;

        // 保持其他寄存器处理不变...
    case APU_SQ1_VOL:
    case APU_SQ1_SWEEP:
    case APU_SQ1_LO:
    case APU_SQ1_HI:
    case APU_SQ2_VOL:
    case APU_SQ2_SWEEP:
    case APU_SQ2_LO:
    case APU_SQ2_HI:
    case APU_TRI_LO:
    case APU_TRI_HI:
    case APU_NOISE_VOL:
    case APU_NOISE_LO:
    case APU_NOISE_HI:
    case APU_DMC_FREQ:
    case APU_DMC_RAW:
    case APU_DMC_START:
    case APU_DMC_LEN:
    case APU_CONTROL:
    case APU_FRAME_CONTROL:
        // 保持原有实现不变...
        break;
    }
}

// 保持readStatus函数不变
Byte APU::readStatus()
{
    bool last_frame_interrupt = frame_counter.frame_interrupt;
    frame_counter.clearFrameInterrupt();
    bool dmc_interrupt = dmc.interrupt;
    dmc.clear_interrupt();
    LOG(CpuTrace) << "APU_STATUS" << std::endl;
    return ((!pulse1.length_counter.muted()) << 0 | (!pulse2.length_counter.muted()) << 1 |
            (!triangle.length_counter.muted()) << 2 | (!noise.length_counter.muted()) << 3 |
            (!dmc.has_more_samples()) << 4 | last_frame_interrupt << 6 | dmc_interrupt << 7);
}

// 修改APU构造函数（假设原有构造函数未初始化音频流）
APU::APU(IRQHandle& irq) 
    : frame_counter(setup_frame_counter(irq)),
      divideByTwo(false)
{
    // 初始化SFML音频流：单声道，44100Hz采样率
    initialize(1, APU_SAMPLE_RATE);
    play(); // 启动音频播放
}

// 保持setup_frame_counter函数不变
FrameCounter APU::setup_frame_counter(IRQHandle& irq)
{
    return FrameCounter(
      {
        std::ref(pulse1.volume),
        std::ref(pulse1.sweep),
        std::ref(pulse1.length_counter),

        std::ref(pulse2.volume),
        std::ref(pulse2.sweep),
        std::ref(pulse2.length_counter),

        std::ref(triangle.length_counter),
        std::ref(triangle.linear_counter),

        std::ref(noise.volume),
        std::ref(noise.length_counter),
      },
      irq);
}
}