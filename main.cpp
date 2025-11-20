#include "src/Pitch.h"
#include "src/RecordPlayer.h"


using HarTy = EnvHarmonics::HarmonicType;
using EnvTy = EnvHarmonics::EnvelopeType;

int main() {
    RecordPlayer player;
    player.start();

    const Pitch p1(
        "C4",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );
    // p1.play();

    const Pitch p2(
        "G4",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );
    // p2.play();

    const Pitch p3(
        "A4",
        1.0f,
        HarTy::PureSine,
        EnvTy::None
    );
    // p3.play();

    std::vector<EnvHarmonics::EnvHar_preset> chord;
    chord.push_back(p1.envHar.preset);
    chord.push_back(p2.envHar.preset);
    chord.push_back(p3.envHar.preset);

    RecordPlayer::trigger(chord);


    player.stop();

    return 0;
}
