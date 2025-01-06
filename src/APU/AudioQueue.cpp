/*#include <stdexcept>*/
/*#include <string>*/
/*#include <cstring>*/
/*#include "APU/AudioQueue.h"*/
/*#include "Log.h"*/
/**/
/*void throwPortAudioError(const PaError err) {*/
/*    if(err == paNoError) {*/
/*        return;*/
/*    }*/
/**/
/*    auto errorMessage = std::string("error: portaudio: ") + std::to_string(err) + Pa_GetErrorText(err);*/
/*    if (err == paUnanticipatedHostError) {*/
/*        const PaHostErrorInfo *hostErrorInfo = Pa_GetLastHostErrorInfo();*/
/*        errorMessage += std::string("; host api error = ") + hostErrorInfo->errorText +*/
/*                "(" + std::to_string(hostErrorInfo->errorCode) + ") apiType = " +*/
/*                std::to_string(hostErrorInfo->hostApiType);*/
/*    }*/
/**/
/*    throw std::runtime_error(errorMessage);*/
/*}*/
/**/
/**/
/*AudioQueue::AudioQueue(int sample_rate, int buffer_size)*/
/*    : m_sample_rate(sample_rate)*/
/*    , buffer(buffer_size)*/
/*{*/
/*    PaError err = Pa_Initialize();*/
/**/
/*    if (err != paNoError) {*/
/*        throwPortAudioError(err);*/
/*    }*/
/**/
/*    m_outputParameters.device = Pa_GetDefaultOutputDevice();*/
/*    if (m_outputParameters.device == paNoDevice) {*/
/*        throw std::runtime_error("can't get default output device");*/
/*    }*/
/**/
/*    m_outputParameters.channelCount = 1;*/
/*    m_outputParameters.sampleFormat = paFloat32;*/
/*    m_outputParameters.suggestedLatency = static_cast<double>(buffer_size) / sample_rate / 2.;*/
/*    m_outputParameters.hostApiSpecificStreamInfo = nullptr;*/
/**/
/*    auto streamCallback = [](*/
/*        const void *input,*/
/*        void *output,*/
/*        unsigned long frameCount,*/
/*        const PaStreamCallbackTimeInfo *timeInfo,*/
/*        PaStreamCallbackFlags statusFlags,*/
/*        void *userData) {*/
/*            return reinterpret_cast<AudioQueue*>(userData)->audioCallback(input, output, frameCount, timeInfo, statusFlags);*/
/*    };*/
/**/
/*    err = Pa_OpenStream(*/
/*        &m_stream,*/
/*        nullptr,*/
/*        &m_outputParameters,*/
/*        static_cast<double>(m_sample_rate),*/
/*        paFramesPerBufferUnspecified,*/
/*        paClipOff,*/
/*        streamCallback,*/
/*        this);*/
/**/
/*    if(err != paNoError) {*/
/*        throwPortAudioError(err);*/
/*    }*/
/**/
/*    const PaStreamInfo* streamInfo = Pa_GetStreamInfo(m_stream);*/
/*    LOG(sn::Info) << "Opening stream with:\n"*/
/*                << "device: " << m_outputParameters.device*/
/*                << ",format: " << m_outputParameters.sampleFormat*/
/*                << ",latency: " << static_cast<int>(1000 * streamInfo->outputLatency) << "ms"*/
/*                << ",sample_rate: " << streamInfo->sampleRate << std::endl;*/
/**/
/*}*/
/**/
/**/
/*AudioQueue::~AudioQueue() {*/
/*    Pa_CloseStream(m_stream);*/
/*    Pa_Terminate();*/
/*}*/
/**/
/*int AudioQueue::audioCallback(*/
/*    [[maybe_unused]] const void *inputBUffer,*/
/*    void *outputBufferRaw,*/
/*    unsigned long framesPerBuffer,*/
/*    [[maybe_unused]] const PaStreamCallbackTimeInfo* timeInfo,*/
/*    [[maybe_unused]] PaStreamCallbackFlags statusFlags) {*/
/**/
/*    const auto outputBuffer = reinterpret_cast<SampleType*>(outputBufferRaw);*/
/*    const auto written = buffer.pop(outputBuffer, framesPerBuffer);*/
/*    // LOG << "Popped " << written <<" from audio buffer. Remaining: " << buffer.size() << std::endl;*/
/**/
/*    if (framesPerBuffer > written && framesPerBuffer - written > 0) {*/
/*        LOG(sn::Info) << "Insufficient audio buffer, had: " << written << ", needed: " << framesPerBuffer << std::endl;*/
/*        std::memset(outputBuffer + written, 0, framesPerBuffer - written);*/
/*    }*/
/**/
/*    return paContinue;*/
/*}*/
/**/
/**/
/*bool AudioQueue::start()*/
/*{*/
/*    if (m_stream == 0)*/
/*        return false;*/
/**/
/*    PaError err = Pa_StartStream(m_stream);*/
/*    LOG(sn::Info) << "Started stream" << std::endl;*/
/*    if (err != paNoError) {*/
/*        throwPortAudioError(err);*/
/*    }*/
/*    return true;*/
/*}*/
/**/
/**/
/*bool AudioQueue::stop()*/
/*{*/
/*    if (m_stream == 0)*/
/*        return false;*/
/**/
/*    PaError err = Pa_StopStream(m_stream);*/
/*    if (err != paNoError) {*/
/*        throwPortAudioError(err);*/
/*    }*/
/*    return true;*/
/*}*/
