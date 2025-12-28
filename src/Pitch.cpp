//
// Created by opus arc on 2025/11/13.
//

#include "Pitch.h"
#include<string>
#include <thread>
#include <utility>
#include <cmath>

#include "RecordPlayer.h"
using namespace std;

void Pitch::play() const {
    RecordPlayer::trigger(envHar.preset);
}

bool Pitch::noteName_formatCheck(const string &input_noteName) {
    // C4 G#5 A6 C7 Bb4 C1
    return
            2 <= input_noteName.size() &&
            input_noteName.size() <= 3 &&

            'A' <= input_noteName[0] &&
            input_noteName[0] <= 'G' &&

            (
                input_noteName.size() <= 2
                    ? '1' <= input_noteName[1] &&
                      input_noteName[1] <= '7'
                    : (
                          input_noteName[1] == '#' ||
                          input_noteName[1] == 'b'
                      )
                      &&
                      '1' <= input_noteName[2] &&
                      input_noteName[2] <= '7'
            );
}

int Pitch::nameToMidi(const string &noteName) {
    char letterName;
    int octaveNumber;
    int accidental;

    if (noteName.size() == 3) {
        letterName = noteName[0];
        octaveNumber = noteName[2] - '0';
        accidental = noteName[1] == '#' ? 1 : -1;
    } else {
        letterName = noteName[0];
        octaveNumber = noteName[1] - '0';
        accidental = 0;
    }

    int _12_TET;

    switch (letterName) {
        case 'C':
            _12_TET = 0 + accidental;
            break;

        case 'D':
            _12_TET = 2 + accidental;
            break;

        case 'E':
            _12_TET = 4 + accidental;
            break;

        case 'F':
            _12_TET = 5 + accidental;
            break;

        case 'G':
            _12_TET = 7 + accidental;
            break;

        case 'A':
            _12_TET = 9 + accidental;
            break;

        case 'B':
            _12_TET = 11 + accidental;
            break;

        default:
            _12_TET = 0;
            break;
    }
    return _12_TET + octaveNumber * 12 + 12;
}

float Pitch::midiToFrequency(const int midi) {
    const int semitoneFromA4 = midi - 69;

    return A4_STANDARD * pow(2.0, static_cast<float>(semitoneFromA4 / 12.0));
}


Pitch::Pitch(
    string input_noteName,
    const float dur,
    const EnvHarmonics::HarmonicType& harTy,
    const EnvHarmonics::EnvelopeType& envTy
) : noteName(std::move(input_noteName)),
    duration(dur),
    midi_n(nameToMidi(noteName)),
    frequency(midiToFrequency(midi_n)),
    envHar(frequency, dur, harTy, envTy) {
}


Pitch::Pitch(const Pitch &input_pitch) = default;

float Pitch::getFrequency(const std::string& noteName) {

    if (!noteName_formatCheck(noteName)) {
        return 0.0f;
    }

    int midi = nameToMidi(noteName);

    return midiToFrequency(midi);
}