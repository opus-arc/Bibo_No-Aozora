cmake_minimum_required(VERSION 4.0)
project(PitchMemory)

set(CMAKE_CXX_STANDARD 20)

add_executable(
        PitchMemory main.cpp
        src/Pitch.cpp
        src/Pitch.h
        External/miniaudio.h
        src/Note.cpp
        src/Note.h
        src/RecordPlayer.cpp
        src/RecordPlayer.h
        Src/TonePreset.h
)
