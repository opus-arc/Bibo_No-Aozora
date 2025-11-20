//
// Created by opus arc on 2025/11/17.
//

#include "EnvHarmonics.h"

#include <iostream>
#include <cmath>

#define PI 3.14159265358979323846f

const EnvHarmonics::EnvHar_preset EnvHarmonics::HE_Preset_SoftPiano{
    .stringPhysicalParams = {
        2.0e-4,
        0.8 /* 阻尼系数 柔和一些，尾巴稍长 */
    },
    .envelope = {
        0, // dur 占位符
        0.007,
        0.3,
        0.6,
        0.8
    },
    .harmonics = {
        {1, 1.00, 1.0, 0.0}, // 基频，比较慢衰减
        {2, 0.60, 2.0, 1.0}, // 高次稍弱
        {3, 0.40, 6.0, -2.0},
        {4, 0.25, 4.0, 3.0},
        {5, 0.18, 5.0, -4.0},
        {6, 0.12, 6.0, 2.0},
    },
    .harFreqCalculate = &EnvHarmonics::pianoHarmonicFreq,
    .envelopeFn = &EnvHarmonics::adsr_singleNoteLinear
};

const EnvHarmonics::EnvHar_preset EnvHarmonics::HE_Preset_Pure_Sine{
    .stringPhysicalParams = {
        2.0e-3,
        0.2 /* 阻尼系数 柔和一些，尾巴稍长 */
    },
    .envelope = {
        0 // dur 占位符
    },
    .harmonics = {
        {1, 1.00, 1.0, 0.0}, // 基频
    },
    .harFreqCalculate = idealizationHarmonicFreq,
    .envelopeFn = nullptr
};


EnvHarmonics::EnvHarmonics(const float f0, const float dur, HarmonicType harTy,
                           EnvelopeType envTy) : fundamentalFre(f0) {
    const EnvHarKey key{harTy, envTy};

    if (const auto it = HE_Preset_Map.find(key); it != HE_Preset_Map.end()) {
        preset = it->second; // 拷贝选中的 preset
        // std::cout <<"\n\n" << fundamentalFre << std::endl;
        preset.state = {
            fundamentalFre,
            dur,
            0.0,
            0.0,
            true
        };
        preset.envelope.duration = dur;
    } else {
        preset = HE_Preset_SoftPiano;
        preset.state = {
            fundamentalFre,
            dur,
            0.0,
            0.0,
            true
        };
        preset.envelope.duration = dur;
        std::cout << "Usage : EnvHarmonics default preset" << std::endl;
    }
};

float EnvHarmonics::synthesizeSample(
    const EnvHar_preset &preset,
    const float fundamentalFreq,
    const float tSec
) {
    // 检查包络值
    // NOLINTNEXTLINE
    const Envelope &env = preset.envelope;
    float envAmp;
    if (
        preset.envelopeFn == nullptr ||
        (env.attack == 0.0f && env.decay == 0.0f && env.sustainLvl == 0.0f && env.release == 0.0f)
    ) {
        envAmp = 1.0f;
    } else {
        // 调用包络计算函数 传入tSec得到当前的振幅
        envAmp = preset.envelopeFn
                     ? preset.envelopeFn(
                         tSec,
                         static_cast<float>(env.duration),
                         static_cast<float>(env.attack),
                         static_cast<float>(env.decay),
                         static_cast<float>(env.sustainLvl),
                         static_cast<float>(env.release)
                     )
                     : 1.0f;

        // 考虑包络有可能错误地计算得到负数的结果 则该帧直接静音
        if (envAmp <= 0.0f)
            return 0.0f;
    }


    // 检查弦的物理参数
    // NOLINTNEXTLINE
    const StringPhysicalParams &sp = preset.stringPhysicalParams;
    const auto B = static_cast<float>(sp.B);

    float sum = 0.0f; // 谐波叠加和
    float norm = 0.0f; // 用于归一化

    // 检查和声信息
    if (preset.harmonics.empty()) {
        return 0.0f;
    }

    // NOLINTNEXTLINE
    for (const auto &h: preset.harmonics) {
        const int n = h.n;
        const auto rel = static_cast<float>(h.relAmplitude);
        const auto damp = static_cast<float>(h.extraDamp);
        const auto detuneCents = static_cast<float>(h.detuneCents);

        // 求出第 n 个谐波的频率（考虑 B，不等间隔谐波）
        float f_n = preset.harFreqCalculate
                        ? preset.harFreqCalculate(fundamentalFreq, n, B)
                        : fundamentalFreq * static_cast<float>(n);

        // 用 cents 做微小失谐：频率乘以 2^(cents/1200)
        if (detuneCents != 0.0f) {
            const float ratio = std::pow(2.0f, detuneCents / 1200.0f);
            f_n *= ratio;
        }

        // 谐波自己的时间衰减
        const float harmonicEnv = (damp > 0.0f)
                                      ? std::exp(-damp * tSec)
                                      : 1.0f;

        const float amp = rel * harmonicEnv;

        // 相位 = 2π f t
        const float phase = 2.0f * PI * f_n * tSec;
        const float value = amp * std::sinf(phase);

        sum += value;
        norm += rel;
    }

    if (norm > 0.0f) {
        sum /= norm; // 归一化，使不同 preset 的总体音量更接近
    }

    // 3. 乘上全局包络
    return sum * envAmp;
}


float EnvHarmonics::idealizationHarmonicFreq(const float f0, const int n, const float placeHolder) {
    const auto nn = static_cast<float>(n);
    return nn * f0;
}

/*
   钢琴式不等间距谐波：f_n = n f0 sqrt(1 + B n^2)
   B = (π³ E r⁴) / (T L²)

   E = 杨氏模量 (Young’s modulus)
   r = 弦半径
   T = 弦张力
   L = 弦有效长度

  Steinway grand example data: 0.0001, 0.0002, 0.00015
  1.5e-4
*/

float EnvHarmonics::pianoHarmonicFreq(const float f0, const int n, const float B) {
    const auto nn = static_cast<float>(n);
    return nn * f0 * std::sqrt(1.0f + B * nn * nn);
}

float EnvHarmonics::adsr_singleNoteLinear(
    const float t,
    const float duration,
    const float attack,
    const float decay,
    const float sustainLvl,
    const float release
) {
    const float sustainStart = attack + decay;
    const float sustainEnd = duration - release;

    if (t < 0.0f) return 0.0f;
    if (t < attack) {
        return t / attack;
    } else if (t < sustainStart) {
        const float x = (t - attack) / decay;
        return 1.0f + (sustainLvl - 1.0f) * x; // 线性从1到sustain
    } else if (t < sustainEnd) {
        return sustainLvl;
    } else if (t < duration) {
        float x = (t - sustainEnd) / release;
        return sustainLvl * (1.0f - x); // 线性衰减到0
    } else {
        return 0.0f;
    }
}
