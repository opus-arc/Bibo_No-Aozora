//
// Created by opus arc on 2025/11/13.
//

#ifndef NOTESPLAYER_PITCH_H
#define NOTESPLAYER_PITCH_H

#include <iostream>
#include<string>

#include "Envelop.h"
#include "Harmonics.h"

using namespace std;
#define A4_STANDARD 440.0


class Pitch {
public:


    string noteName;
    int midi_n;
    float frequency;
    float duration;

    Envelop envelope;
    Harmonics harmonics;


    // "D#2" and *player
    explicit Pitch(
        string input_noteName,
        float duration,
        const Envelop& input_envelope,
        const Harmonics& harmonic
    );

    // Pitch = Pitch
    Pitch(const Pitch &input_pitch);

    void play() const;

private:
    static bool noteName_formatCheck(const string &input_noteName);

    static int nameToMidi(const string &noteName);

    static float midiToFrequency(int midi);
};

#endif //NOTESPLAYER_PITCH_H
