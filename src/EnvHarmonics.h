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
        PureSine,
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
    using HarmonicFreqCalculate = float(*)(float f0, int n, float);
    HarmonicFreqCalculate harmonicFreq = nullptr;

    static float idealizationHarmonicFreq(float f0, int n, float placeHolder);
    static float pianoHarmonicFreq(float f0, int n, float B);

    // Envelop -------------------------------------------
    // -------Structs---------
    enum EnvelopeType {
        None,
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
    static const EnvHar_preset HE_Preset_Pure_Sine;

    std::map<EnvHarKey, EnvHar_preset> HE_Preset_Map = {
        {{PureSine, None}, HE_Preset_Pure_Sine},
        {{SoftPiano, SingleNoteLinear}, HE_Preset_SoftPiano}
    };

    EnvHar_preset preset;

    EnvHarmonics(float f0, float dur, HarmonicType harTy, EnvelopeType envTy);

    static float synthesizeSample(const EnvHar_preset &preset, float fundamentalFreq, float tSec);
};


#endif //BIBO_NO_AOZORA_HARMONIC_H
