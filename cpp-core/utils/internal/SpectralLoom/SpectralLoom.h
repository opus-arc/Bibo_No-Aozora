//
// Created by opus arc on 2026/1/23.
//

#ifndef BIBO_NO_AOZORA_SPECTRALLOOM_H
#define BIBO_NO_AOZORA_SPECTRALLOOM_H
#include "tools/ComprehensiveExamination.h"

#include "SpectralLoom.h"
#include "tools/CelloDynamic.h"


// 根据条件自动生成乐句音频
class SpectralLoom {

    public:
    SpectralLoom();
    ~SpectralLoom();

    static void playInterval(const std::string &interval);
};

#endif //BIBO_NO_AOZORA_SPECTRALLOOM_H