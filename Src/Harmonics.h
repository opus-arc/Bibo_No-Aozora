//
// Created by opus arc on 2025/11/17.
//

#ifndef BIBO_NO_AOZORA_HARMONIC_H
#define BIBO_NO_AOZORA_HARMONIC_H
#include <iostream>
#include <vector>


class Harmonics {
public:
    struct Harmonic {
        int n; // 第 n 个谐波
        double relAmplitude; // 相对强度
        double extraDamp; // 额外阻尼
        double detuneCents; // 轻微失谐（增加厚度）
    };
    enum HarmonicType {
      SoftPiano
    };

    std::vector<Harmonic> flavor;

    explicit Harmonics(const HarmonicType type) {
        switch (type) {
            case SoftPiano:
                flavor = softHarmonics;
                break;
            default:
                flavor = softHarmonics;
                std::cout << "Usage : default harmonics"<<std::endl;
        }
    }

    std::vector<Harmonic> softHarmonics = {
        {1, 1.00, 1.0,   0.0},  // 基频，比较慢衰减
        {2, 0.60, 2.0,   1.0},  // 高次稍弱
        {3, 0.40, 3.0,  -2.0},
        {4, 0.25, 4.0,   3.0},
        {5, 0.18, 5.0,  -4.0},
        {6, 0.12, 6.0,   2.0},
    };
};



#endif //BIBO_NO_AOZORA_HARMONIC_H