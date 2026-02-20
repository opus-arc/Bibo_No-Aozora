#include "MidiFile.h"
#include <iostream>

using namespace smf;

int main() {
    MidiFile mf;

    mf.absoluteTicks();
    mf.setTicksPerQuarterNote(480);

    const int track   = 0;
    const int channel = 0;
    const int tpq     = mf.getTicksPerQuarterNote();

    // Tempo / 拍号（尽量让读谱软件更开心）
    mf.addTempo(track, 0, 120.0);
    mf.addTimeSignature(track, 0, 4, 4);   // 4/4（midifile 支持这个接口）

    // Program Change: Acoustic Grand Piano
    mf.addPatchChange(track, 0, channel, 0);

    int tick = 0;
    const int vel = 100;
    const int dur = tpq; // 四分音符

    int notes[] = {60, 62, 64, 65, 67, 69, 71, 72};

    for (int i = 0; i < 8; i++) {
        mf.addNoteOn(track, tick, channel, notes[i], vel);
        mf.addNoteOff(track, tick + dur, channel, notes[i]);
        tick += dur;
    }

    // End-of-Track（有些软件对这个很挑剔）
    mf.addMetaEvent(track, tick + 1, 0x2F, "");

    mf.sortTracks();

    const std::string out = "test.mid";
    bool ok = mf.write(out);
    if (!ok) {
        std::cerr << "MIDI write failed: " << out << "\n";
        return 1;
    }
    std::cout << "Wrote: " << out << "\n";
    return 0;
}