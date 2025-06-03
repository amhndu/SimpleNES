#include "AudioPlayer.h"
#include "Log.h"
#include "miniaudio.h"

namespace sn
{
void data_callback(ma_device*                   device,
                   void*                        output,
                   [[maybe_unused]] const void* input,
                   ma_uint32                    required_output_frame_count)
{
    if (device->pUserData == nullptr)
    {
        return;
    }

    CallbackData& cb_data = *(CallbackData*)device->pUserData;

    if (cb_data.mute)
    {
        return;
    }

    if (cb_data.remaining_buffer_rounds-- > 0)
    {
        LOG(sn::Info) << "skipping buffer round" << std::endl;
        return;
    }

    ma_uint64 presample_input_frames = 0;
    ma_result result                 = ma_resampler_get_required_input_frame_count(
      cb_data.resampler, required_output_frame_count, &presample_input_frames);
    if (result != MA_SUCCESS)
    {
        presample_input_frames =
          required_output_frame_count * cb_data.resampler->sampleRateIn / cb_data.resampler->sampleRateOut;
        LOG(sn::Error) << "ma_resampler_get_required_input_frame_count failed: " << result << std::endl;
    }

    cb_data.input_frames_buffer.resize(presample_input_frames);
    ma_uint64 presample_frames_avail =
      cb_data.ring_buffer.pop(cb_data.input_frames_buffer.data(), presample_input_frames);
    // copy the last sample
    if (presample_frames_avail < presample_input_frames && presample_frames_avail > 0)
    {
        LOG(Info) << "insufficient presample frames" << VAR_PRINT(presample_frames_avail)
                  << VAR_PRINT(presample_input_frames) << std::endl;
        for (auto idx = presample_frames_avail; idx < presample_input_frames; ++idx)
        {
            cb_data.input_frames_buffer[idx] = cb_data.input_frames_buffer[presample_frames_avail];
        }
    }

    ma_uint64 output_frame_count64 = required_output_frame_count;
    ma_resampler_process_pcm_frames(cb_data.resampler,
                                    reinterpret_cast<void*>(cb_data.input_frames_buffer.data()),
                                    &presample_input_frames,
                                    output,
                                    &output_frame_count64);
    if (result != MA_SUCCESS)
    {
        LOG(sn::Error) << "resampling failed; errorcode=" << result << std::endl;
    }
}

bool AudioPlayer::start()
{
    deviceConfig                          = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format          = ma_format_f32;
    deviceConfig.playback.channels        = 1;
    deviceConfig.sampleRate               = output_sample_rate;
    deviceConfig.dataCallback             = data_callback;
    deviceConfig.pUserData                = &cb_data;
    deviceConfig.periodSizeInMilliseconds = callback_period_ms.count();

    auto result                           = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS)
    {
        LOG(Error) << "Failed to open playback device: error code = " << result << std::endl;
        return false;
    }

    result = ma_device_start(&device);
    if (result != MA_SUCCESS)
    {
        LOG(Error) << "Failed to start playback device: error code = " << result << std::endl;
        ma_device_uninit(&device);
        return false;
    }

    ma_resampler_config config = ma_resampler_config_init(ma_format_f32,
                                                          deviceConfig.playback.channels,
                                                          input_sample_rate,
                                                          output_sample_rate,
                                                          ma_resample_algorithm_linear);
    result                     = ma_resampler_init(&config, nullptr, &resampler);
    if (result != MA_SUCCESS)
    {
        LOG(Error) << "Failed to start playback device: error code = " << result << std::endl;
        ma_device_uninit(&device);
        return false;
    }

    initialized = true;
    return true;
}

AudioPlayer::~AudioPlayer()
{
    if (!initialized)
    {
        return;
    }

    ma_device_uninit(&device);
}

void AudioPlayer::mute()
{
    cb_data.mute = true;
}

}
