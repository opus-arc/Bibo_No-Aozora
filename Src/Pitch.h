//
// Created by opus arc on 2025/11/13.
//

#ifndef NOTESPLAYER_PITCH_H
#define NOTESPLAYER_PITCH_H

#include <iostream>
#include<string>

#include "RecordPlayer.h"
using namespace std;
#define A4_STANDARD 440.0


class Pitch {
    string noteName = "A4";

    static bool noteName_formatCheck(const string &input_noteName);

    int midi_n = 0;
    float frequency = A4_STANDARD;

    void noteToFrequency();

    RecordPlayer *player;

public:
    explicit Pitch(const string &input_noteName);

    explicit Pitch();

    Pitch(const Pitch &input_pitch);

    void playTheSound() const;

    void print_noteName() const;

    void print_frequency() const;

    void midi_n_frequency() const;

    string get_noteName();

    [[nodiscard]] float get_frequency() const;

    [[nodiscard]] int get_midi_n() const;
};


#endif //NOTESPLAYER_PITCH_H
