//
// Created by opus arc on 2026/1/23.
//

#ifndef BIBO_NO_AOZORA_SPECTRALLOOM_H
#define BIBO_NO_AOZORA_SPECTRALLOOM_H
#include "tools/ComprehensiveExamination.h"

// 根据条件自动生成乐句音频

class SpectralLoom {

    public:

    // 乐句考试
    static void CE(const std::string &level) {
        ComprehensiveExamination comprehensiveExamination(level);
    }

    SpectralLoom();
    ~SpectralLoom();
};

#endif //BIBO_NO_AOZORA_SPECTRALLOOM_H