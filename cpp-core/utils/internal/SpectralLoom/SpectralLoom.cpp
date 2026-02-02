//
// Created by opus arc on 2026/1/23.
//

#include "SpectralLoom.h"

#include "tools/RecordPlayer.h"

void SpectralLoom::playInterval(const std::string &interval) {
    static const std::unordered_map<std::string, std::string> upperNote = {
        {"unison", "C2"},   // 0
        {"min2",   "C#2"},  // 1
        {"maj2",   "D2"},   // 2
        {"min3",   "D#2"},  // 3
        {"maj3",   "E2"},   // 4
        {"per4",     "F2"},   // 5
        {"dim5",   "F#2"},  // 6
        {"per5",     "G2"},   // 7
        {"min6",   "G#2"},  // 8
        {"maj6",   "A2"},   // 9
        {"min7",   "A#2"},  // 10
        {"maj7",   "B2"},   // 11
        {"octave", "C2"}    // 12
    };

    RecordPlayer player;
    player.start();

    const auto it = upperNote.find(interval);
    if (it != upperNote.end()) {
        CelloDynamic::play("C2",       0.833f);
        CelloDynamic::play(it->second, 0.833f);
    }

    player.stop();
}
