#include "AudioPlayer.h"
#include "Log.h"

namespace sn {
    void data_callback(ma_device* pDevice, void* pOutput, [[maybe_unused]] const void* pInput, ma_uint32 frameCount) {
        if (pDevice->pUserData == nullptr) {
            return;
        }

        CallbackData& cbData = *(CallbackData*)pDevice->pUserData;

        if (cbData.remaining_buffer_rounds-- > 0) {
            LOG(sn::Info) << "skipping buffer round" << std::endl;
            return;
        }

        auto framesAvail = cbData.ring_buffer.pop(reinterpret_cast<float*>(pOutput), frameCount);

        if (framesAvail < frameCount) {
            LOG(sn::Info) << "not enough data; emitting zeroes; avail: " << framesAvail << "; frameCount: " << frameCount << std::endl;
            float *fOutput = reinterpret_cast<float*>(pOutput);
            for (size_t remaining = 0; remaining < frameCount - framesAvail; ++remaining) {
                fOutput[remaining + framesAvail] = 0;
            }
        }
    }

    bool AudioPlayer::start() {
        deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.format   = ma_format_f32; // TODO: experiment with ints
        deviceConfig.playback.channels = 1;
        deviceConfig.sampleRate        = std_sample_rate;
        deviceConfig.dataCallback      = data_callback;
        deviceConfig.pUserData         = &cb_data;
        deviceConfig.periodSizeInMilliseconds = 100;

        auto result = ma_device_init(NULL, &deviceConfig, &device);
        if (result != MA_SUCCESS) {
            LOG(Error) << "Failed to open playback device: error code = " << result << std::endl;
            return false;
        }

        result = ma_device_start(&device);
        if (result != MA_SUCCESS) {
            LOG(Error) << "Failed to start playback device: error code = " << result << std::endl;
            ma_device_uninit(&device);
            return false;
        }
        
        initialized = true;
        return true;
    }

    AudioPlayer::~AudioPlayer() {
        if (!initialized) {
            return;
        }

        ma_device_uninit(&device);
    }
}
