//
// Created by opus arc on 2025/11/17.
//

#ifndef BIBO_NO_AOZORA_ENVELOP_H
#define BIBO_NO_AOZORA_ENVELOP_H
#include <iostream>


/*
 *  basic_recordPlayer_setting
 *  fre dur har env
 *
 */


class Envelop {
public:
    enum EnvelopeType {
        Normal
    };

    using EnvelopeFn = double(*)(
        double t,
        double duration,
        double attack,
        double decay,
        double sustainLevel,
        double release
    );

    EnvelopeFn customFun = nullptr;

    explicit Envelop(const EnvelopeType type) {
        switch (type) {
            case Normal:
                customFun = adsr_normal;
                break;
            default:
                customFun = adsr_normal;
                std::cout << "Usage: default envelop"<<std::endl;
        }
    }


    static double adsr_normal(
        const double t,
        const double totalDuration,
        const double attack,
        const double decay,
        const double sustainLevel,
        const double release
    ) {
        const double sustainStart = attack + decay;
        const double sustainEnd = totalDuration - release;

        if (t < 0.0) return 0.0;
        if (t < attack) {
            return t / attack;
        } else if (t < sustainStart) {
            double x = (t - attack) / decay;
            return 1.0 + (sustainLevel - 1.0) * x; // 线性从1到sustain
        } else if (t < sustainEnd) {
            return sustainLevel;
        } else if (t < totalDuration) {
            double x = (t - sustainEnd) / release;
            return sustainLevel * (1.0 - x); // 线性衰减到0
        } else {
            return 0.0;
        }
    }
};


#endif //BIBO_NO_AOZORA_ENVELOP_H
