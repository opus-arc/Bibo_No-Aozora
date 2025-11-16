//
// Created by opus arc on 2025/11/14.
//

// This definition must before the miniaudio.h
#define MINIAUDIO_IMPLEMENTATION
#include "RecordPlayer.h"

#include <iostream>
#include <thread>

// Basic Setting
#define AUDIO_SAMPLE_RATE      48000   // 采样率：48.0kHz
#define AUDIO_CHANNELS         1       // 声道数：2 = 立体声
#define AUDIO_FORMAT           ma_format_f32  // 使用 float
#define AUDIO_MASTER_VOLUME    1.0f    // 全局主音量（最后一道乘法）

#define AUDIO_FRAMES_PER_BUF   512     // buffer 大小，影响延迟/稳定
#define AUDIO_BACKEND_DEFAULT  1       // 使用默认后端

// Audio Quality
#define SYNTH_WAVE_DEFAULT   WAVE_SINE   // 默认正弦波形
#define SYNTH_CLIP_LIMIT     0.98f       // 防止爆音的上限
#define SYNTH_MAX_VOICES     8          // 同时最多叠加的声音个数
#define SYNTH_ENABLE_PAN     1          // 是否需要支持左右声道偏移

// Basic Constants
#define Pai 3.14159265358979323846f


RecordPlayer::RecordPlayer(const ToneState &tone_state) {
    config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = AUDIO_FORMAT;
    config.playback.channels = AUDIO_CHANNELS;
    config.sampleRate = AUDIO_SAMPLE_RATE;
    config.dataCallback = dataCallback; // 或者你自己的 callback
    // Initialize tone timing state (step 1: fixed duration demo)
    gTone.frequency = tone_state.frequency; // keep using current frequency source
    gTone.duration = tone_state.duration; // default to 1 second for now
    gTone.elapsed = tone_state.elapsed;
    gTone.phase = tone_state.phase;
    gTone.active = tone_state.active;
    config.pUserData = &gTone;

    if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio device.\n";
        return;
    }
}

RecordPlayer::RecordPlayer() {
    config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = AUDIO_FORMAT;
    config.playback.channels = AUDIO_CHANNELS;
    config.sampleRate = AUDIO_SAMPLE_RATE;
    config.dataCallback = dataCallback; // 或者你自己的 callback
    // Initialize tone timing state (step 1: fixed duration demo)
    gTone.frequency = 440.0f; // keep using current frequency source
    gTone.duration = 1.0f; // default to 1 second for now
    gTone.elapsed = 0.0f;
    gTone.phase = 0.0f;
    gTone.active = true;
    config.pUserData = &gTone;

    if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio device.\n";
        return;
    }
}

RecordPlayer::~RecordPlayer() {
    ma_device_uninit(&device);
}

void RecordPlayer::start() {
    ma_result r = ma_device_start(&device);
    if (r != MA_SUCCESS)
        std::cerr << "[RecordPlayer] Failed to start device, ma_result=" << r << "\n";
    while (gTone.active) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void RecordPlayer::stop() {
    ma_device_stop(&device);
}

// can't motify this sign it's not flexible
void RecordPlayer::dataCallback(
    ma_device *pDevice,
    void *pOutput,
    const void *pInput,
    const ma_uint32 frameCount
) {
    auto *out = static_cast<float *>(pOutput);
    auto *t = static_cast<ToneState *>(pDevice->pUserData);

    const float phaseIncrement = 2.0f * Pai * t->frequency / AUDIO_SAMPLE_RATE;
    const float dt = 1.0f / static_cast<float>(AUDIO_SAMPLE_RATE);

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        float sample = 0.0f;

        if (t->active && t->elapsed < t->duration) {
            sample = sinf(t->phase);
            t->phase += phaseIncrement;
            if (t->phase >= 2.0f * Pai) {
                t->phase -= 2.0f * Pai;
            }
            t->elapsed += dt;

            if (t->elapsed >= t->duration) {
                t->active = false; // finished; subsequent frames will be silent
            }
        } else {
            sample = 0.0f; // silent after duration
        }

        for (int ch = 0; ch < AUDIO_CHANNELS; ++ch) {
            *out++ = sample;
        }
    }
}
