//
// Created by opus arc on 2025/11/14.
//

#ifndef PITCHMEMORY_PLAYER_H
#define PITCHMEMORY_PLAYER_H

// Basic Setting
#define AUDIO_SAMPLE_RATE      44100   // 采样率：44.1kHz
#define AUDIO_CHANNELS         2       // 声道数：2 = 立体声
#define AUDIO_FORMAT           ma_format_f32  // 使用 float
#define AUDIO_MASTER_VOLUME    1.0f    // 全局主音量（最后一道乘法）

#define AUDIO_FRAMES_PER_BUF   512     // buffer 大小，影响延迟/稳定
#define AUDIO_BACKEND_DEFAULT  1       // 使用默认后端

// Audio Quality
#define SYNTH_WAVE_DEFAULT   WAVE_SINE   // 默认正弦波形
#define SYNTH_CLIP_LIMIT     0.98f       // 防止爆音的上限
#define SYNTH_MAX_VOICES     8          // 同时最多叠加的声音个数
#define SYNTH_ENABLE_PAN     1          // 是否需要支持左右声道偏移

#define MINIAUDIO_IMPLEMENTATION

struct Note {
    float freq      = 440.0f;
    float volume    = 0.7f;
    float duration  = 1.0f;

    float attack    = 0.01f;
    float decay     = 0.1f;
    float sustain   = 0.8f;
    float release   = 0.2f;

    float pan       = 0.0f;   // -1 ~ 1 平衡
    int   waveType  = 0;      // 不展开
};


#endif //PITCHMEMORY_PLAYER_H