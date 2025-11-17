//
// Created by opus arc on 2025/11/14.
//

#ifndef PITCHMEMORY_PLAYER_H
#define PITCHMEMORY_PLAYER_H


#include "Pitch.h"
#include "../External/miniaudio.h"


class RecordPlayer{
public:
    explicit RecordPlayer();

    ~RecordPlayer();

    void start();

    // static void blockUntilSilence() ;

    static void trigger(const Pitch& p);

    void stop();

private:
    ma_device device{};
    ma_device_config config{};


    static void dataCallback(
        ma_device *pDevice,
        void *pOutput,
        const void *pInput,
        ma_uint32 frameCount
    );
};

#endif // PITCHMEMORY_PLAYER_H
