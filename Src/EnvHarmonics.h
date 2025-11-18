//
// Created by opus arc on 2025/11/17.
//

#ifndef BIBO_NO_AOZORA_HARMONIC_H
#define BIBO_NO_AOZORA_HARMONIC_H

#include <map>
#include <vector>



class EnvHarmonics {
public:

    float fundamentalFre{};

    // Harmonics ----------------------------------------

    // -------Structs---------
    enum HarmonicType {
        SoftPiano
    };

    struct SingleHar {
        int n; // 第 n 个谐波
        float relAmplitude; // 相对强度
        float extraDamp; // 额外阻尼
        float detuneCents; // 轻微失谐（增加厚度）
    };

    struct StringPhysicalParams {
        float B;
        float globalDamp;
    };

    std::vector<SingleHar> harmonics;

    // -------FuncPoints------
    using HarmonicFreqCalculate = float(*)(float f0, int n, float B);
    HarmonicFreqCalculate harmonicFreq = nullptr;

    static float pianoHarmonicFreq(float f0, int n, float B);

    // Envelop -------------------------------------------
    // -------Structs---------
    enum EnvelopeType {
        SingleNoteLinear
    };
    struct Envelope {
        float duration;
        float attack;
        float decay;
        float sustainLvl;
        float release;
    };

    // -------FuncPoints------
    using EnvelopeFn = float(*)(float t, float duration, float attack, float decay,
                                float sustainLvl, float release);
    EnvelopeFn customFun = nullptr;

    static float adsr_singleNoteLinear(float t, float duration, float attack, float decay,
                                       float sustainLvl, float release);

    // EnvHarmonics ------------------------------------
    struct RuntimeState {
        float frequency;
        float duration;
        float elapsed;
        float phase;
        bool active;
    };
    struct EnvHar_preset {
        RuntimeState state;
        StringPhysicalParams stringPhysicalParams;
        Envelope envelope;
        std::vector<SingleHar> harmonics;

        HarmonicFreqCalculate harFreqCalculate;
        EnvelopeFn envelopeFn;
    };

    using EnvHarKey = std::pair<HarmonicType, EnvelopeType>;

    static const EnvHar_preset HE_Preset_SoftPiano;

    std::map<EnvHarKey, EnvHar_preset> HE_Preset_Map = {
        {{SoftPiano, SingleNoteLinear}, HE_Preset_SoftPiano}
    };

    EnvHar_preset preset;

    EnvHarmonics(float f0, HarmonicType harTy, EnvelopeType envTy);

    static float synthesizeSample(const ::EnvHarmonics::EnvHar_preset &preset, float fundamentalFreq, float tSec);
};


#endif //BIBO_NO_AOZORA_HARMONIC_H
