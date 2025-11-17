#include "Src/Pitch.h"
#include "Src/RecordPlayer.h"

int main() {
    RecordPlayer player;
    player.start();
    Pitch p(
        "C5",
        1.0f,
        Envelop(Envelop::Normal),
        Harmonics(Harmonics::SoftPiano)
        );
    p.play();

    player.stop();
}

