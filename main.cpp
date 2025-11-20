#include "src/Pitch.h"
#include "src/RecordPlayer.h"


using HarTy = EnvHarmonics::HarmonicType;
using EnvTy = EnvHarmonics::EnvelopeType;

int main() {
    RecordPlayer player;
    player.start();

    const Pitch p(
        "C4",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );
    p.play();

    const Pitch p2(
        "G4",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );
    p2.play();

    const Pitch p3(
        "G4",
        1.0f,
        HarTy::PureSine,
        EnvTy::None
    );
    p3.play();


    player.stop();

    return 0;
}
