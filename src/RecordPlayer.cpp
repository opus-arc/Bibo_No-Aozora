//
// Created by opus arc on 2025/11/14.
//

// This definition must before the miniaudio.h
#define MINIAUDIO_IMPLEMENTATION
#include "RecordPlayer.h"
#include "Pitch.h"

#include <iostream>
#include <thread>

// Audio Quality
#define AUDIO_SAMPLE_RATE      48000.0f       // 采样率：48.0kHz
#define SYNTH_WAVE_DEFAULT     WAVE_SINE      // 默认正弦波形
#define AUDIO_CHANNELS         1              // 声道数：2 = 立体声
#define AUDIO_FORMAT           ma_format_f32  // 使用 float
#define AUDIO_MASTER_VOLUME    1.0f           // 全局主音量（最后一道乘法）
#define AUDIO_FRAMES_PER_BUF   512            // buffer 大小，影响延迟/稳定

// Basic Setting
#define AUDIO_BACKEND_DEFAULT  1              // 使用默认后端
#define SYNTH_CLIP_LIMIT       0.98f          // 防止爆音的上限
#define SYNTH_MAX_VOICES       8              // 同时最多叠加的声音个数
#define SYNTH_ENABLE_PAN       1              // 是否需要支持左右声道偏移

// Basic Constants
#define PI     3.14159265358979323846f
#define E      2.71828182845904523536f


std::vector<EnvHarmonics::EnvHar_preset> presets;


RecordPlayer::RecordPlayer() {
    config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = AUDIO_FORMAT;
    config.playback.channels = AUDIO_CHANNELS;
    config.sampleRate = AUDIO_SAMPLE_RATE;
    config.dataCallback = dataCallback;
    // Initialize tone timing state (step 1: fixed duration demo)

    config.pUserData = &presets;

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
    if (r != MA_SUCCESS) {
        std::cerr << "[RecordPlayer] Failed to start device, ma_result=" << r << "\n";
    }
}

void RecordPlayer::stop() {
    ma_device_stop(&device);
}

void RecordPlayer::trigger(const EnvHarmonics::EnvHar_preset &envHar_pre) {
    // 拷贝预设到设备用的 preset
    presets.push_back(envHar_pre);

    // 重置运行时状态
    presets.back().state.duration = presets.back().envelope.duration; // 播放时长按包络 duration
    presets.back().state.elapsed = 0.0f;
    presets.back().state.phase = 0.0f;
    presets.back().state.active = true;

    // 简单给设备线程一点时间看到新数据
    while (presets.back().state.active) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // std::cout<<preset.state.frequency<<" "<<preset.state.phase<<"\n";
}

void RecordPlayer::trigger(const std::vector<EnvHarmonics::EnvHar_preset> &pres) {
    // 拷贝预设到设备用的 preset
    presets = pres;

    // 重置运行时状态
    for (auto &p: presets) {
        p.state.duration = p.envelope.duration; // 播放时长按包络 duration
        p.state.elapsed = 0.0f;
        p.state.phase = 0.0f;
        p.state.active = true;
    }
        // 简单给设备线程一点时间看到新数据，直到所有音符播放结束
        bool anyActive = true;
        while (anyActive) {
            anyActive = false;
            for (const auto &p: presets) {
                if (p.state.active) {
                    anyActive = true;
                    break;
                }
            }
            if (anyActive) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // std::cout<<preset.state.frequency<<" "<<preset.state.phase<<"\n";
        }

}

void RecordPlayer::dataCallback(
    ma_device *pDevice,
    void *pOutput,
    const void *pInput,
    const ma_uint32 frameCount
) {
    auto *out = static_cast<float *>(pOutput);
    auto *presetsPtr = static_cast<std::vector<EnvHarmonics::EnvHar_preset> *>(pDevice->pUserData);

    const auto sampleRate = static_cast<float>(pDevice->sampleRate);

    // 一帧多少秒
    const float dt = 1.0f / sampleRate;

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        float sample = 0.0f;

        if (!presetsPtr->empty()) {
            float sum   = 0.0f;
            float count = 0.0f;
            for (auto &pre : *presetsPtr) {
                auto &state = pre.state;
                if (state.active && state.elapsed < state.duration) {
                    // 用谐波 + 包络合成当前时刻的一个 sample
                    const float s = EnvHarmonics::synthesizeSample(
                        pre,
                        state.frequency,
                        state.elapsed
                    );

                    sum   += s;
                    count += 1.0f;

                    state.elapsed += dt;
                    if (state.elapsed >= state.duration) {
                        state.active = false; // 播放结束，转为静音
                    }
                    if (count > 0.0f) {
                        sample = sum / count;   // 简单平均混音
                    } else {
                        sample = 0.0f;          // 所有 voice 都静音
                    }
                }
            }
        } else {
            // cout<<"没有 preset 数据，静音"<<endl;
            sample = 0.0f; // 没有 preset 数据，静音
        }

        // 写入所有输出声道
        for (int ch = 0; ch < pDevice->playback.channels; ++ch) {
            *out++ = sample;
        }
    }
}
