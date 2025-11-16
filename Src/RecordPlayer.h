//
// Created by opus arc on 2025/11/14.
//

#ifndef PITCHMEMORY_PLAYER_H
#define PITCHMEMORY_PLAYER_H

#include "../External/miniaudio.h"

// Simple tone state to support duration timing
struct ToneState {
    mutable float frequency;   // Hz
    float duration;    // seconds
    float elapsed;     // seconds
    float phase;       // radians
    bool  active;      // whether the tone is still sounding
};

static ToneState gTone; // one global tone for now (step 1: duration only)

class RecordPlayer{
public:
    explicit RecordPlayer();
    explicit RecordPlayer(const ToneState& tone_state);

     ~RecordPlayer();

     void start();
private:


    ma_device device{};
    ma_device_config config{};

    void stop();

    static void dataCallback(
        ma_device *pDevice,
        void *pOutput,
        const void *pInput,
        ma_uint32 frameCount
    );
};

#endif // PITCHMEMORY_PLAYER_H
