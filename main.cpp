#include "Src/Pitch.h"
#include "Src/RecordPlayer.h"


using HarTy = EnvHarmonics::HarmonicType;
using EnvTy = EnvHarmonics::EnvelopeType;

int main() {
    RecordPlayer player;
    player.start();

    const Pitch p(
        "C5",
        1.0f,
        HarTy::SoftPiano,
        EnvTy::SingleNoteLinear
    );
    p.play();

    const Pitch p2(
    "G5",
    1.0f,
    HarTy::SoftPiano,
    EnvTy::SingleNoteLinear
);
    p2.play();


    player.stop();

    return 0;
}
