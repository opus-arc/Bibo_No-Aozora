// //
// // Created by opus arc on 2025/11/14.
// //
//
// #ifndef PITCHMEMORY_TONEPRESET_H
// #define PITCHMEMORY_TONEPRESET_H
//
// #include "Pitch.h"
//
//
// // enum class Flavor {
// //     Bell,
// //     Piano,
// //     Violin,
// //     Guitar
// // };
//
// struct SingleTrack {
//     enum class WaveType {
//         Sine, // 纯正弦 → 最干净
//         Square, // 方波 → 带奇次谐波 → 电子音
//         Saw, // 锯齿波 → 带所有谐波 → 亮、刺
//         Noise, // 纯噪声 → 鼓、风、冲击
//         Custom // 自定义波形（依赖谐波表）
//     };
//
//     struct ADSR {
//         float attack; // 音量从 0 到峰值时间
//         float decay; // 从峰值降到 sustain 的时间
//         float sustain; // 持续阶段的音量 0~1
//         float release; // 松开后衰减到 0 的时间
//
//         ADSR(const float a, const float d, const float s, const float r)
//             : attack(a), decay(d), sustain(s), release(r) {
//         };
//
//         ADSR() : attack(0.0f), decay(0.0f), sustain(1.0), release(0.0f) {
//         };
//     };
//
//     struct ToneDressUp {
//         float vibratoDepth; // 颤音幅度（Hz）
//         float vibratoRate; // 颤音速度（Hz）
//         float noiseAmount; // 混入噪声比例
//         float detune; // 多声部微调（厚度）
//
//         ToneDressUp(const float vDepth, const float vRate, const float noise, const float d)
//             : vibratoDepth(vDepth), vibratoRate(vRate), noiseAmount(noise), detune(d) {
//         }
//
//         ToneDressUp() : vibratoDepth(0.05f), vibratoRate(5.0f), noiseAmount(0.1f), detune(0.0f) {
//         }
//     };
//
//     Pitch pitch = Pitch("A4"); // 基音 音高
//     WaveType waveType = WaveType::Sine; // 波形图
//     ADSR adsr = ADSR(); // 包络数据
//     ToneDressUp toneDressUp = ToneDressUp(); // 细节修饰
//
//     SingleTrack() : pitch(),
//                     waveType(),
//                     adsr(),
//                     toneDressUp() {
//     }
//
//     SingleTrack(
//         const Pitch &pitch,
//
//         const WaveType waveType,
//         const ADSR &adsr,
//         const ToneDressUp &toneDressUp
//     ) : pitch(pitch),
//         waveType(waveType),
//         adsr(adsr),
//         toneDressUp(toneDressUp) {
//     }
// };
//
// // struct Harmonic {
// //     float freqRatio; // 泛音（如 1.0, 2.5, 3.8）
// //     float amp; // 强度（0~1）振幅
// //
// //     Harmonic(
// //         const float freqRatio,
// //         const float amplitude
// //     )
// //         : freqRatio(freqRatio), amp(amplitude) {
// //     }
// //
// //     Harmonic() {
// //         freqRatio = 1.0f;
// //         amp = 1.0f;
// //     }
// // };
//
//
// // struct TonePreset_CONFIG {
// //     WaveType waveType = WaveType::Sine;
// //     ADSR adsr = ADSR();
// //     ToneDressUp toneDressUp = ToneDressUp();
// //     Harmonic harmonic = Harmonic();
// //
// //     TonePreset_CONFIG(
// //         WaveType wa,
// //         ADSR ad,
// //         ToneDressUp to,
// //         Harmonic ha
// //     ) : waveType(WaveType::Sine),
// //                     {
// //     }
// // };
//
// // inline unordered_map<Flavor, TonePreset_CONFIG> tonePresets = {
// //     {
// //         Flavor::Bell, {
// //
// //         }
// //     },
// //
// // };
//
// // struct TonePreset {
// //     Pitch fundamentalPitch;
// //     float fundamentalFrequency;
// //
// //     WaveType waveType; // 该基音的波形图
// //     ADSR adsr; // 该基音的包络数据
// //     ToneDressUp toneDressUp; // 该基音的细节修饰
// //
// //     std::vector<Harmonic> harmonics; // 该基因的泛音群设计
// //
// //     TonePreset(const string &pitchName, const Flavor &flavorName)
// //         : fundamentalPitch(),
// //           fundamentalFrequency(),
// //           waveType(),
// //           adsr(),
// //           toneDressUp(),
// //           harmonics() {
// //         switch (flavorName) {
// //             case Flavor::Bell: {
// //                 // 基音：用正弦 + 铃铛型包络
// //                 waveType = WaveType::Sine;
// //                 adsr = ADSR(0.01f, 0.3f, 0.0f, 2.0f);
// //                 toneDressUp = ToneDressUp(0.05f, 5.0f, 0.1f, 0.0f);
// //
// //                 harmonics.clear();
// //                 // 一个简单的“铃铛泛音群”示例，你可以之后再微调这些数字
// //                 harmonics.emplace_back(1.0f, 1.0f); // 基频
// //                 harmonics.emplace_back(2.7f, 0.5f); // 泛音 1
// //                 harmonics.emplace_back(4.1f, 0.3f); // 泛音 2
// //                 harmonics.emplace_back(5.2f, 0.15f); // 泛音 3
// //                 break;
// //             }
// //             case Flavor::Piano: {
// //                 waveType = WaveType::Sine;
// //                 adsr = ADSR(0.01f, 0.2f, 0.6f, 0.5f);
// //                 toneDressUp = ToneDressUp(0.0f, 0.0f, 0.0f, 0.0f);
// //                 harmonics.clear();
// //                 break;
// //             }
// //             case Flavor::Violin: {
// //                 waveType = WaveType::Saw;
// //                 adsr = ADSR(0.1f, 0.3f, 0.8f, 0.5f);
// //                 toneDressUp = ToneDressUp(0.1f, 6.0f, 0.0f, 0.02f);
// //                 harmonics.clear();
// //                 break;
// //             }
// //             case Flavor::Guitar: {
// //                 waveType = WaveType::Square;
// //                 adsr = ADSR(0.01f, 0.2f, 0.3f, 1.0f);
// //                 toneDressUp = ToneDressUp(0.02f, 5.0f, 0.05f, 0.0f);
// //                 harmonics.clear();
// //                 break;
// //             }
// //             default:
// //                 break;
// //         }
// //     }
// // };
// #endif //PITCHMEMORY_TONEPRESET_H
