//
// Created by mac coiler on 2025/12/12.
//

#include "CelloDynamic.h"
#include "Pitch.h"
#include "RecordPlayer.h"
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm> // for std::max

using namespace std;

// ==========================================================
// Cello-Dynamic_policy
// ==========================================================
void CelloDynamic::play(const string& noteName, float musicalDuration) {

    float freq = Pitch::getFrequency(noteName);
    if (freq <= 0.0f) {
        cerr << "Invalid Note: " << noteName << endl;
        return;
    }

    // -------------------------------------------------------------
    // The Bowing Policy
    // -------------------------------------------------------------
    float dynamicAttack;
    float dynamicDecay;
    float dynamicSustain = 0.90f; // 默认 Sustain

    // (Prestissimo): < 0.15s
    // "Spiccato" (跳弓)
    if (musicalDuration < 0.15f) {
        dynamicAttack = 0.015f;
        dynamicDecay  = 0.05f;
        dynamicSustain = 1.0f;
    }
    //  (Allegro): < 0.40s
    // "Détaché" (分弓)
    else if (musicalDuration < 0.40f) {
        dynamicAttack = 0.05f;
        dynamicDecay  = 0.10f;
    }
    // (Adagio/Lento): >= 0.40s
    // "Legato" (连弓)
    else {
        dynamicAttack = 0.20f;
        dynamicDecay  = 0.10f;
    }

    const float releaseTime = (musicalDuration < 0.15f) ? 0.30f : 0.40f;

    // 物理总时长 = 手按时间 + 离弦尾音
    float physicalDuration = musicalDuration + releaseTime;

    EnvHarmonics::EnvHar_preset preset;

    preset.state.frequency = freq;
    preset.state.duration = physicalDuration;
    preset.state.active = true;
    preset.state.elapsed = 0.0f;

    preset.envelope = {
        physicalDuration,
        dynamicAttack,    // <--- 关键优化
        dynamicDecay,     // <--- 关键优化
        dynamicSustain,
        releaseTime
    };

    // 物理参数 (B值稍微调大一点点，增加极速演奏时的擦弦粗糙感)
    preset.stringPhysicalParams = { 2.5e-5f, 0.4f };

    // 谐波表 (保持 Audition 数据)
    preset.harmonics = {
        {1, 1.00f, 1.0f, 0.0f},
        {2, 0.82f, 1.0f, 0.5f}, //H2：大提琴听起来厚实、温暖、有木头味的主要原因
        {3, 0.47f, 1.2f, -0.5f},
        {4, 0.20f, 1.5f, 0.8f},
        {5, 0.22f, 1.8f, -0.8f},//共振峰
        {6, 0.24f, 2.0f, 1.0f},//共振峰
        {7, 0.16f, 3.0f, -1.2f},
        {8, 0.10f, 5.0f, 1.5f},
        {12,0.05f, 8.0f, 0.0f}
    };
    //共振峰 (Formant)：大提琴的琴体木板在 500-600Hz 附近有固有共振。有了这个“凸起”，声音才像真琴。

    //  滤波器参数 (待实现)
    preset.harFreqCalculate = &EnvHarmonics::pianoHarmonicFreq;
    preset.envelopeFn = &EnvHarmonics::adsr_singleNoteLinear;
    //preset.lpf_cutoff = 1500.0f;
    //preset.lpf_envAmount = (musicalDuration < 0.15f) ? 2.0f : 1.5f; // 模拟弓毛刚接触琴弦时粗糙的摩擦声

    RecordPlayer::trigger(preset);

    float sleepTime;

    if (musicalDuration < 0.15f) {
        // 【极速模式等待】
        // 对于极短的音，睡掉 85% 的时间，留 15% 给下一个音重叠
        sleepTime = musicalDuration * 0.85f;
    } else {
        // 【普通模式等待】
        // 固定重叠 0.04秒
        sleepTime = std::max(0.0f, musicalDuration - 0.04f);
    }

    this_thread::sleep_for(chrono::milliseconds(static_cast<int>(sleepTime * 1000)));
}
