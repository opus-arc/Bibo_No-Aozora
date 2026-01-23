//
// Created by mac coiler on 2025/12/12.
//

#ifndef BIBO_NO_AOZORA_CELLODYNAMIC_H
#define BIBO_NO_AOZORA_CELLODYNAMIC_H
#include <string>
#include "EnvHarmonics.h"


class CelloDynamic {
public:
    static void play(const std::string& noteName, float musicalDuration);
private:
    static EnvHarmonics::EnvHar_preset createDynamicPreset(float freq, float phyiscalDuration, float attackTime);
};


#endif //BIBO_NO_AOZORA_CELLODYNAMIC_H