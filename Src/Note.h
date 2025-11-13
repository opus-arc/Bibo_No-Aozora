//
// Created by opus arc on 2025/11/13.
//

#ifndef PITCHMEMORY_NOTE_H
#define PITCHMEMORY_NOTE_H

#include "Pitch.h"


class Note{
public:
    Note();
    ~Note();
private:
    Pitch pitch;  // C4 G#5 A6 C7 Bb4 C1

    float freq      = 440.0f;
    float volume    = 0.7f;
    float duration  = 1.0f;

    float attack    = 0.01f;
    float decay     = 0.1f;
    float sustain   = 0.8f;
    float release   = 0.2f;

    float pan       = 0.0f;   // -1 ~ 1 平衡
    int   waveType  = 0;      // 不展开
};


#endif //PITCHMEMORY_NOTE_H