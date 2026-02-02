#include "SpectralLoom/RecordPlayer.h"
#include "SpectralLoom/Pitch.h"
#include "SpectralLoom/CelloDynamic.h"
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using HarTy = EnvHarmonics::HarmonicType;
using EnvTy = EnvHarmonics::EnvelopeType;

int _main() {
    RecordPlayer player;
    player.start();

    cout << ">>> Start Concert <<<" << endl;

    // -------------------------------------------------
    // 1.CelloDynamic solo
    // -------------------------------------------------
    cout << "[Cello Solo]" << endl;
    CelloDynamic::play("G2", 0.833f);
    CelloDynamic::play("D2", 0.633f);
    CelloDynamic::play("B2", 0.766f);
    CelloDynamic::play("A2", 0.700f);
    CelloDynamic::play("E2", 0.900f);
    CelloDynamic::play("G2", 0.833f);

    this_thread::sleep_for(chrono::milliseconds(1000));


    // -------------------------------------------------
    // 2. Cello + Chord
    // -------------------------------------------------
    cout << "[Ensemble]" << endl;

    const Pitch p1(
        "C3",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );

    const Pitch p2(
        "E3",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );

    const Pitch p3(
        "G3",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );

    const Pitch p4(
        "A3",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );

    const Pitch p5(
    "B3",
    1.0f,
    HarTy::SoftPiano,
    EnvTy::SingleNoteLinear
);
    vector<EnvHarmonics::EnvHar_preset> chord;
    chord.push_back(p1.envHar.preset);
    chord.push_back(p2.envHar.preset);
    chord.push_back(p3.envHar.preset);

    RecordPlayer::trigger(chord);

    chord.push_back(p1.envHar.preset);
    chord.push_back(p3.envHar.preset);
    chord.push_back(p4.envHar.preset);

    RecordPlayer::trigger(chord);

    chord.push_back(p1.envHar.preset);
    chord.push_back(p2.envHar.preset);
    chord.push_back(p4.envHar.preset);
    chord.push_back(p5.envHar.preset);

    RecordPlayer::trigger(chord);

    CelloDynamic::play("G1", 0.15f);
    CelloDynamic::play("D2", 0.09f);
    CelloDynamic::play("B2", 0.09f);
    CelloDynamic::play("A2", 0.132f);
    CelloDynamic::play("B2", 0.09f);
    CelloDynamic::play("D2", 0.09f);
    CelloDynamic::play("B2", 0.13f);
    CelloDynamic::play("D2", 0.173f);

    CelloDynamic::play("G1", 0.15f);
    CelloDynamic::play("D2", 0.09f);
    CelloDynamic::play("B2", 0.09f);
    CelloDynamic::play("A2", 0.132f);
    CelloDynamic::play("B2", 0.09f);
    CelloDynamic::play("D2", 0.09f);
    CelloDynamic::play("B2", 0.13f);
    CelloDynamic::play("D2", 0.173f);

    CelloDynamic::play("G1", 0.132f);
    CelloDynamic::play("E2", 0.073f);
    CelloDynamic::play("C3", 0.083f);
    CelloDynamic::play("B2", 0.13f);
    CelloDynamic::play("C3", 0.071f);
    CelloDynamic::play("E2", 0.13f);
    CelloDynamic::play("C3", 0.073f);
    CelloDynamic::play("E2", 0.23f);

    CelloDynamic::play("G1", 0.132f);
    CelloDynamic::play("E2", 0.073f);
    CelloDynamic::play("C3", 0.083f);
    CelloDynamic::play("B2", 0.13f);
    CelloDynamic::play("C3", 0.071f);
    CelloDynamic::play("E2", 0.13f);
    CelloDynamic::play("C3", 0.073f);
    CelloDynamic::play("E2", 0.23f);

    CelloDynamic::play("G1", 0.132f);
    CelloDynamic::play("F#2", 0.073f);
    CelloDynamic::play("C3", 0.083f);
    CelloDynamic::play("B2", 0.13f);
    CelloDynamic::play("C3", 0.071f);
    CelloDynamic::play("F#2", 0.13f);
    CelloDynamic::play("C3", 0.073f);
    CelloDynamic::play("F#2", 0.23f);

    CelloDynamic::play("G1", 0.132f);
    CelloDynamic::play("F#2", 0.073f);
    CelloDynamic::play("C3", 0.083f);
    CelloDynamic::play("B2", 0.13f);
    CelloDynamic::play("C3", 0.071f);
    CelloDynamic::play("F#2", 0.13f);
    CelloDynamic::play("C3", 0.073f);
    CelloDynamic::play("F#2", 0.23f);

    CelloDynamic::play("G2", 0.132f);
    CelloDynamic::play("G3", 0.073f);
    CelloDynamic::play("B2", 0.083f);
    CelloDynamic::play("A2", 0.13f);
    CelloDynamic::play("B2", 0.071f);
    CelloDynamic::play("G2", 0.13f);
    CelloDynamic::play("B2", 0.073f);
    CelloDynamic::play("G2", 0.23f);

    CelloDynamic::play("G1", 0.132f);
    CelloDynamic::play("G2", 0.073f);
    CelloDynamic::play("B2", 0.083f);
    CelloDynamic::play("A2", 0.13f);
    CelloDynamic::play("B2", 0.071f);
    CelloDynamic::play("G2", 0.13f);
    CelloDynamic::play("B2", 0.073f);
    CelloDynamic::play("F#2", 0.23f);

    CelloDynamic::play("G1", 0.132f);
    CelloDynamic::play("E2", 0.073f);
    CelloDynamic::play("B2", 0.083f);
    CelloDynamic::play("A2", 0.13f);
    CelloDynamic::play("B2", 0.071f);
    CelloDynamic::play("G2", 0.13f);
    CelloDynamic::play("F#2", 0.073f);
    CelloDynamic::play("G2", 0.23f);

    CelloDynamic::play("E2", 0.132f);
    CelloDynamic::play("G2", 0.073f);
    CelloDynamic::play("F#2", 0.083f);
    CelloDynamic::play("G2", 0.13f);
    CelloDynamic::play("B1", 0.071f);
    CelloDynamic::play("D2", 0.13f);
    CelloDynamic::play("C#2", 0.073f);
    CelloDynamic::play("B1", 0.23f);

    CelloDynamic::play("C#2", 0.073f);
    CelloDynamic::play("G2", 0.23f);
    CelloDynamic::play("A2", 0.083f);
    CelloDynamic::play("G2", 0.13f);
    CelloDynamic::play("A2", 0.083f);
    CelloDynamic::play("G2", 0.13f);
    CelloDynamic::play("A2", 0.083f);
    CelloDynamic::play("G2", 0.13f);

    CelloDynamic::play("C#2", 0.073f);
    CelloDynamic::play("G2", 0.13f);
    CelloDynamic::play("A2", 0.083f);
    CelloDynamic::play("G2", 0.13f);
    CelloDynamic::play("A2", 0.083f);
    CelloDynamic::play("G2", 0.073f);
    CelloDynamic::play("A2", 0.083f);
    CelloDynamic::play("G2", 0.13f);

    CelloDynamic::play("F#2", 0.132f);
    CelloDynamic::play("A2", 0.073f);
    CelloDynamic::play("D3", 0.053f);
    CelloDynamic::play("C#3", 0.073f);
    CelloDynamic::play("D3", 0.051f);
    CelloDynamic::play("A2", 0.13f);
    CelloDynamic::play("G2", 0.073f);
    CelloDynamic::play("A2", 0.23f);

    CelloDynamic::play("F#2", 0.132f);
    CelloDynamic::play("A2", 0.073f);
    CelloDynamic::play("G2", 0.083f);
    CelloDynamic::play("A2", 0.13f);
    CelloDynamic::play("D2", 0.071f);
    CelloDynamic::play("F#2", 0.13f);
    CelloDynamic::play("E2", 0.073f);
    CelloDynamic::play("D2", 0.23f);

    CelloDynamic::play("E1", 0.73f);

    this_thread::sleep_for(chrono::milliseconds(1000));

    player.stop();
    cout << ">>> Concert Finished <<<" << endl;

    return 0;
}
