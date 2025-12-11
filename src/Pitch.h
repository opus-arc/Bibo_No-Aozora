//
// Created by opus arc on 2025/11/13.
//

#ifndef NOTESPLAYER_PITCH_H
#define NOTESPLAYER_PITCH_H

#include <iostream>
#include <string>

#include "EnvHarmonics.h"

using namespace std;
#define A4_STANDARD 440.0


class Pitch {
public:


    string noteName;
    float duration;
    int midi_n;
    float frequency;

    EnvHarmonics envHar;


    // "D#2" and *player
    explicit Pitch(
        string input_noteName,
        float dur,
        const EnvHarmonics::HarmonicType& harTy,
        const EnvHarmonics::EnvelopeType& envTy
    );

    // Pitch = Pitch
    Pitch(const Pitch &input_pitch);

    void play() const;

    static float getFrequency(const std::string& noteName);
private:
    static bool noteName_formatCheck(const string &input_noteName);

    static int nameToMidi(const string &noteName);

    static float midiToFrequency(int midi);
};

#endif //NOTESPLAYER_PITCH_H
