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
        0.02,
        0.3,
        0.6,
        0.8
    },
    .harmonics = {
        {1, 1.00, 1.0, 0.0}, // 基频，比较慢衰减
        {2, 0.60, 2.0, 2.0}, // 高次稍弱
        {3, 0.45, 6.0, -4.0},
        {4, 0.15, 4.0, 6.0},
        {5, 0.18, 5.0, -8.0},
        {6, 0.02, 6.0, 4.0}
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

const EnvHarmonics::EnvHar_preset EnvHarmonics::HE_Preset_Cello {
    .stringPhysicalParams = {
        2.0e-5f,
        0.4f
    },

    .envelope = {
        0.0f,    // 占位符
        0.20f,   // 慢起音
        0.10f,   // Decay
        0.90f,   // Sustain
        0.40f    // 长尾音
    },

    .harmonics = {
                {1, 1.00f, 1.0f, 0.0f},
                {2, 0.82f, 1.0f, 0.5f},
                {3, 0.47f, 1.2f, -0.5f},
                {4, 0.20f, 1.5f, 0.8f},
                {5, 0.22f, 1.8f, -0.8f},
                {6, 0.24f, 2.0f, 1.0f},
                {7, 0.16f, 3.0f, -1.2f},
                {8, 0.10f, 5.0f, 1.5f},
                {12,0.05f, 8.0f, 0.0f}
    },

    .harFreqCalculate = &EnvHarmonics::pianoHarmonicFreq,
    .envelopeFn = &EnvHarmonics::adsr_singleNoteLinear,

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
}


// 如果你的工程已经有 PI 定义可以删掉这一行
constexpr float JIT_PI = 3.14159265358979323846f;

/*
 * 返回值：抖动后的频率（乘法偏移）
 *
 * 参数：
 * - baseFreq: 经 detune 等静态处理后的谐波基频（Hz）
 * - harmonicIndex: 谐波序号 n（从 1 开始）
 * - tSec: 当前时间（秒）
 * - globalSigma: 全局抖动强度（相对值），例如 0.001f 表示约 ±0.1% 频率偏移
 * - harmonicJitterScale: 随谐波级数增长的放大系数，例如 0.02f
 * - lfoFreqs: 指向一组 LFO 基频（Hz），用来叠加成平滑抖动
 * - numLFOs: lfoFreqs 数量
 *
 * 说明：函数使用若干固定低频正弦（LFO）叠加并对相位引入与谐波序号相关的伪随机初相，
 *       因此同一谐波在不同时间点轨迹可复现；且不需要在外部保存状态。
 */
const float jitterLFOs[3] = {0.18f, 0.41f, 0.73f}; // Hz
inline float applyJitterByLFOS(
    float baseFreq,
    int harmonicIndex,
    float tSec,
    float globalSigma,
    float harmonicJitterScale,
    const float *lfoFreqs,
    int numLFOs
) {
    if (numLFOs <= 0 || lfoFreqs == nullptr) return baseFreq;

    // 简单整数哈希 -> [0,1) -> * 2π，生成可复现的初相
    auto pseudoPhase = [](int key, int idx) -> float {
        unsigned int x = static_cast<unsigned int>(key);
        x = (x ^ 61u) ^ (x >> 16);
        x = x + (x << 3);
        x = x ^ (x >> 4);
        x = x * 0x27d4eb2d;
        x = x ^ (x >> 15);
        x ^= static_cast<unsigned int>(idx * 0x9e3779b9u);
        const float v = static_cast<float>(x & 0xFFFFFFu) / static_cast<float>(0x1000000u); // [0,1)
        return v * 2.0f * JIT_PI;
    };

    // 叠加多个低频正弦，产生平滑抖动（近似滤波白噪）
    float acc = 0.0f;
    for (int i = 0; i < numLFOs; ++i) {
        // 可以微微依赖谐波序号避免所有谐波完全同步
        float lfoFreq = lfoFreqs[i] * (1.0f + 0.03f * static_cast<float>(harmonicIndex));
        float phi0 = pseudoPhase(harmonicIndex, i);
        acc += std::sinf(2.0f * JIT_PI * lfoFreq * tSec + phi0);
    }

    // 归一化到大致 [-1,1]（numLFOs 个正弦的平均）
    acc /= static_cast<float>(numLFOs);

    // 抖动幅度按谐波数放大（常见：高次谐波更不稳定）
    const float sigma = globalSigma * (1.0f + harmonicJitterScale * static_cast<float>(harmonicIndex));

    // multiplicative 抖动（1 + δ），δ 在大约 ±sigma 范围
    float delta = acc * sigma;

    return baseFreq * (1.0f + delta);
}

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


        // 调用频率抖动模块
        f_n = applyJitterByLFOS(
            f_n,
            n, tSec,
            0.002f,
            0.02f,
            jitterLFOs,
            3
        );

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


float EnvHarmonics::synthesizeSample(
    const std::vector<EnvHar_preset> &presets,
    const float fundamentalFreq,
    const float tSec
) {
    if (presets.empty()) return 0.0f;

    float sum = 0.0f;
    float norm = 0.0f;

    for (const auto &preset: presets) {
        if (preset.harmonics.empty()) continue;

        const float v = synthesizeSample(preset, fundamentalFreq, tSec);
        sum += v;
        norm += 1.0f;
    }

    if (norm > 0.0f) {
        sum /= norm;
    }

    return sum;
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

  Steinway grand example mapper: 0.0001, 0.0002, 0.00015
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
